// Copyright 2018 Emre Simsirli

#include "Board.h"
#include "BoardDefs.h"
#include "PosKey.h"
#include "Piece.h"
#include "Square.h"
#include "Undo.h"
#include "Verify.h"
#include "Side.h"
#include "Debug.h"

#define MAX_POSITION_MOVES 256

#define HASH_PIECE(p, sq)   (pos_key_ ^= PosKey::GetPieceKey(p, sq))
#define HASH_CASTL()        (pos_key_ ^= PosKey::GetCastleKey(cast_perm_))
#define HASH_SIDE()         (pos_key_ ^= PosKey::GetSideKey())
#define HASH_EN_P()         (pos_key_ ^= PosKey::GetPieceKey(EPieceType::empty, en_passant_sq_))

#define INFINITE 30000
#define MATE 29000

constexpr auto start_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

namespace
{
    const int32 PawnTable[64] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        10, 10, 0, -10, -10, 0, 10, 10,
        5, 0, 0, 5, 5, 0, 0, 5,
        0, 0, 10, 20, 20, 10, 0, 0,
        5, 5, 5, 10, 10, 5, 5, 5,
        10, 10, 10, 20, 20, 10, 10, 10,
        20, 20, 20, 30, 30, 20, 20, 20,
        0, 0, 0, 0, 0, 0, 0, 0
    };

    const int32 KnightTable[64] = {
        0, -10, 0, 0, 0, 0, -10, 0,
        0, 0, 0, 5, 5, 0, 0, 0,
        0, 0, 10, 10, 10, 10, 0, 0,
        0, 0, 10, 20, 20, 10, 5, 0,
        5, 10, 15, 20, 20, 15, 10, 5,
        5, 10, 10, 20, 20, 10, 10, 5,
        0, 0, 5, 10, 10, 5, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
    };

    const int32 BishopTable[64] = {
        0, 0, -10, 0, 0, -10, 0, 0,
        0, 0, 0, 10, 10, 0, 0, 0,
        0, 0, 10, 15, 15, 10, 0, 0,
        0, 10, 15, 20, 20, 15, 10, 0,
        0, 10, 15, 20, 20, 15, 10, 0,
        0, 0, 10, 15, 15, 10, 0, 0,
        0, 0, 0, 10, 10, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
    };

    const int32 RookTable[64] = {
        0, 0, 5, 10, 10, 5, 0, 0,
        0, 0, 5, 10, 10, 5, 0, 0,
        0, 0, 5, 10, 10, 5, 0, 0,
        0, 0, 5, 10, 10, 5, 0, 0,
        0, 0, 5, 10, 10, 5, 0, 0,
        0, 0, 5, 10, 10, 5, 0, 0,
        25, 25, 25, 25, 25, 25, 25, 25,
        0, 0, 5, 10, 10, 5, 0, 0
    };

    const uint32 Mirror[64] = {
        56, 57, 58, 59, 60, 61, 62, 63,
        48, 49, 50, 51, 52, 53, 54, 55,
        40, 41, 42, 43, 44, 45, 46, 47,
        32, 33, 34, 35, 36, 37, 38, 39,
        24, 25, 26, 27, 28, 29, 30, 31,
        16, 17, 18, 19, 20, 21, 22, 23,
        8, 9, 10, 11, 12, 13, 14, 15,
        0, 1, 2, 3, 4, 5, 6, 7
    };

    const uint32 castle_perm[120] = {
        15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
        15, 13, 15, 15, 15, 12, 15, 15, 14, 15,
        15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
        15, 7, 15, 15, 15, 3, 15, 15, 11, 15,
        15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15, 15, 15
    };
}

TBoard::TBoard()
{
    set(start_fen);
}

void TBoard::reset()
{
    for(auto& sq : b_)
        sq = ESquare::offboard;

    for(uint32 sq = 0; sq < n_board_squares; ++sq)
        b_[ESquare::Sq120(sq)] = EPieceType::empty;

    for(auto& pawns : pawns_)
        pawns.Empty();

    for(uint32 i = 0; i < 2; ++i) {
        n_big_pieces_[i] = 0;
        n_major_pieces_[i] = 0;
        n_minor_pieces_[i] = 0;
        material_score_[i] = 0;
    }

    for(auto& locs : piece_locations_)
        locs.SetNum(0);

    for(auto& k_sq : king_sq_)
        k_sq = ESquare::no_sq;

    history_.Reset();
    side_ = ESide::both;
    en_passant_sq_ = ESquare::no_sq;
    fifty_move_counter_ = 0;
    ply_ = 0;
    cast_perm_ = 0;
    pos_key_ = 0;
}

bool TBoard::set(const FString& fen)
{
    reset();

    auto f = *fen;
    for(int32 rank = ERank::rank_8, file = EFile::file_a; rank >= ERank::rank_1 && *f; f++) {
        uint32 count = 1;
        auto piece = EPieceType::empty;
        switch(*f) {
            case 'p':
                piece = EPieceType::bp;
                break;
            case 'r':
                piece = EPieceType::br;
                break;
            case 'n':
                piece = EPieceType::bn;
                break;
            case 'b':
                piece = EPieceType::bb;
                break;
            case 'q':
                piece = EPieceType::bq;
                break;
            case 'k':
                piece = EPieceType::bk;
                break;
            case 'P':
                piece = EPieceType::wp;
                break;
            case 'R':
                piece = EPieceType::wr;
                break;
            case 'N':
                piece = EPieceType::wn;
                break;
            case 'B':
                piece = EPieceType::wb;
                break;
            case 'Q':
                piece = EPieceType::wq;
                break;
            case 'K':
                piece = EPieceType::wk;
                break;

            case '1': case '2': case '3':
            case '4': case '5': case '6':
            case '7': case '8': {
                count = *f - '0';
                break;
            }

            case '/': case ' ': {
                rank--;
                file = EFile::file_a;
                continue;
            }

            default:
                return false;
        }

        for(uint32 i = 0; i < count; ++i) {
            const auto sq120 = ESquare::Sq120(file, rank);
            if(piece != EPieceType::empty)
                b_[sq120] = piece;
        }

        file += count;
    }

    MAKE_SURE(*f == 'w' || *f == 'b');

    side_ = *f == 'w' ? ESide::white : ESide::black;
    f += 2;

    for(uint32 i = 0; i < 4; ++i) {
        if(*f == ' ')
            break;

        switch(*f) {
            case 'K':
                cast_perm_ |= ECastlingPermission::c_wk;
                break;
            case 'Q':
                cast_perm_ |= ECastlingPermission::c_wq;
                break;
            case 'k':
                cast_perm_ |= ECastlingPermission::c_bk;
                break;
            case 'q':
                cast_perm_ |= ECastlingPermission::c_bq;
                break;
            default:
                break;
        }
        f++;
    }
    f++;

    MAKE_SURE(cast_perm_ >= 0 && cast_perm_ < 16);
    MAKE_SURE(*f != ' ');

    if(*f != '-') {
        const uint32 file = f[0] - 'a';
        const uint32 rank = f[1] - '1';

        MAKE_SURE(file >= EFile::file_a && file <= EFile::file_h);
        MAKE_SURE(rank >= ERank::rank_1 && rank <= ERank::rank_8);

        en_passant_sq_ = ESquare::Sq120(file, rank);
    }

    pos_key_ = generate_pos_key();
    update_material();
    return true;
}

uint64 TBoard::generate_pos_key()
{
    uint64 key = 0;
    for(uint32 sq = 0; sq < n_board_squares_x; ++sq) {
        const auto p = b_[sq];
        if(p != ESquare::no_sq && p != ESquare::offboard && p != EPieceType::empty) {
            MAKE_SURE(p >= EPieceType::wp && p <= EPieceType::bk);
            key ^= PosKey::GetPieceKey(p, sq);
        }
    }

    if(side_ == ESide::white) {
        key ^= PosKey::GetSideKey();
    }

    if(en_passant_sq_ != ESquare::no_sq) {
        MAKE_SURE(en_passant_sq_ >= 0 && en_passant_sq_ < n_board_squares_x);
        key ^= PosKey::GetPieceKey(EPieceType::empty, en_passant_sq_);
    }

    MAKE_SURE(cast_perm_ >= 0 && cast_perm_ < 16);
    key ^= PosKey::GetCastleKey(cast_perm_);
    return key;
}

void TBoard::update_material()
{
    for(uint32 sq = 0; sq < n_board_squares_x; ++sq) {
        const auto piece = b_[sq];
        if(piece != ESquare::offboard && piece != EPieceType::empty) {
            const auto p = pieces[piece];
            const auto side = p.side;

            if(p.is_big) {
                n_big_pieces_[side]++;
                if(p.is_major)
                    n_major_pieces_[side]++;
                else if(p.is_minor)
                    n_minor_pieces_[side]++;
            }

            material_score_[side] += p.value;
            piece_locations_[piece].Add(sq);

            switch(piece) {
                case EPieceType::wk:
                    king_sq_[ESide::white] = sq;
                    break;
                case EPieceType::bk:
                    king_sq_[ESide::black] = sq;
                    break;
                case EPieceType::wp:
                    pawns_[ESide::white].SetSquare(ESquare::Sq64(sq));
                    pawns_[ESide::both].SetSquare(ESquare::Sq64(sq));
                    break;
                case EPieceType::bp:
                    pawns_[ESide::black].SetSquare(ESquare::Sq64(sq));
                    pawns_[ESide::both].SetSquare(ESquare::Sq64(sq));
                    break;
                default: break;
            }
        }
    }
}

TArray<TMove> TBoard::generate_moves()
{
    return move_generator_.generate_moves();
}

TArray<TMove> TBoard::generate_moves(const uint32 sq)
{
    return move_generator_.generate_moves(sq);
}

bool TBoard::make_move(const TMove& m)
{
    MAKE_SURE(is_valid());

    const auto from = m.from();
    const auto to = m.to();
    const auto side = side_;

    MAKE_SURE(Verification::IsSquareOnBoard(from));
    MAKE_SURE(Verification::IsSquareOnBoard(to));
    MAKE_SURE(Verification::IsSideValid(side));
    MAKE_SURE(Verification::IsPieceValid(b_[from]));

    auto u = FUndo(m);
    u.pos_key = pos_key_;

    if(m.is_enpassant()) {
        if(side == ESide::white) {
            clear_piece(to - 10);
        } else {
            clear_piece(to + 10);
        }
    } else if(m.is_castling()) {
        switch(to) {
            case ESquare::c1:
                move_piece(ESquare::a1, ESquare::d1);
                break;
            case ESquare::c8:
                move_piece(ESquare::a8, ESquare::d8);
                break;
            case ESquare::g1:
                move_piece(ESquare::h1, ESquare::f1);
                break;
            case ESquare::g8:
                move_piece(ESquare::h8, ESquare::f8);
                break;
            default:
            verify(false);
        }
    }

    if(en_passant_sq_ != ESquare::no_sq)
        HASH_EN_P();
    HASH_CASTL();

    u.fifty_move_counter = fifty_move_counter_;
    u.en_passant_sq = en_passant_sq_;
    u.cast_perm = cast_perm_;

    cast_perm_ &= castle_perm[from];
    cast_perm_ &= castle_perm[to];
    en_passant_sq_ = ESquare::no_sq;

    HASH_CASTL();

    fifty_move_counter_++;

    const auto captured = m.captured_piece();
    if(captured != EPieceType::empty) {
        MAKE_SURE(Verification::IsPieceValid(captured));
        clear_piece(to);
        fifty_move_counter_ = 0;
    }

    ply_++;
    history_.Add(u);

    if(pieces[b_[from]].is_pawn) {
        fifty_move_counter_ = 0;
        if(m.is_pawnstart()) {
            if(side == ESide::white) {
                en_passant_sq_ = from + 10;
                MAKE_SURE(ESquare::Rank(en_passant_sq_) == ERank::rank_3);
            } else {
                en_passant_sq_ = from - 10;
                MAKE_SURE(ESquare::Rank(en_passant_sq_) == ERank::rank_6);
            }
            HASH_EN_P();
        }
    }

    move_piece(from, to);

    const auto promoted = m.promoted_piece();
    if(promoted != EPieceType::empty) {
        MAKE_SURE(Verification::IsPieceValid(promoted) && !pieces[promoted].is_pawn);
        clear_piece(to);
        add_piece(to, promoted);
    }

    if(pieces[b_[to]].is_king) {
        king_sq_[side] = to;
    }

    side_ ^= 1;
    HASH_SIDE();

    MAKE_SURE(is_valid());

    if(is_attacked(king_sq_[side], side_)) {
        take_move();
        return false;
    }

    return true;
}

void TBoard::take_move()
{
    MAKE_SURE(is_valid());

    auto h = history_.Pop();
    ply_--;

    const auto from = h.move.from();
    const auto to = h.move.to();
    MAKE_SURE(Verification::IsSquareOnBoard(from));
    MAKE_SURE(Verification::IsSquareOnBoard(to));

    if(en_passant_sq_ != ESquare::no_sq)
        HASH_EN_P();
    HASH_CASTL();

    cast_perm_ = h.cast_perm;
    fifty_move_counter_ = h.fifty_move_counter;
    en_passant_sq_ = h.en_passant_sq;

    if(en_passant_sq_ != ESquare::no_sq)
        HASH_EN_P();
    HASH_CASTL();

    side_ ^= 1;
    HASH_SIDE();

    if(h.move.is_enpassant()) {
        if(side_ == ESide::white) {
            add_piece(to - 10, EPieceType::bp);
        } else {
            add_piece(to + 10, EPieceType::wp);
        }
    } else if(h.move.is_castling()) {
        switch(to) {
            case ESquare::c1: move_piece(ESquare::d1, ESquare::a1);
                break;
            case ESquare::c8: move_piece(ESquare::d8, ESquare::a8);
                break;
            case ESquare::g1: move_piece(ESquare::f1, ESquare::h1);
                break;
            case ESquare::g8: move_piece(ESquare::f8, ESquare::h8);
                break;
            default:
            verify(false);
        }
    }

    move_piece(to, from);

    if(pieces[b_[from]].is_king) {
        king_sq_[side_] = from;
    }

    const auto captured = h.move.captured_piece();
    if(captured != EPieceType::empty) {
        MAKE_SURE(Verification::IsPieceValid(captured));
        add_piece(to, captured);
    }

    const auto promoted = h.move.promoted_piece();
    if(h.move.is_promoted()) {
        MAKE_SURE(Verification::IsPieceValid(promoted) && !pieces[promoted].is_pawn);
        clear_piece(from);
        add_piece(from, pieces[promoted].side == ESide::white ? EPieceType::wp : EPieceType::bp);
    }

    MAKE_SURE(is_valid());
}

bool TBoard::move_exists(const TMove& m)
{
    const auto moves = generate_moves();
    for(auto& move : moves) {
        if(!make_move(move))
            continue;
        take_move();
        if(move == m)
            return true;
    }
    return false;
}

bool TBoard::is_attacked(const uint32 sq, const uint8 attacking_side) const
{
    MAKE_SURE(Verification::IsSquareOnBoard(sq));
    MAKE_SURE(Verification::IsSideValid(attacking_side));
    MAKE_SURE(is_valid());

    // pawns
    if(attacking_side == ESide::white) {
        if(b_[sq - 11] == wp || b_[sq - 9] == wp)
            return true;
    } else {
        if(b_[sq + 11] == bp || b_[sq + 9] == bp)
            return true;
    }

    // knights
    for(auto dir : pieces[wn].move_directions) {
        const auto p = b_[sq + dir];
        const auto piece = pieces[p];
        if(p != ESquare::offboard && piece.is_knight && piece.side == attacking_side)
            return true;
    }

    // rooks & queens
    for(auto dir : pieces[wr].move_directions) {
        auto sqq = sq + dir;
        auto piece = b_[sqq];
        while(piece != ESquare::offboard) {
            if(piece != empty) {
                if(pieces[piece].is_rook_queen && pieces[piece].side == attacking_side)
                    return true;
                break;
            }
            sqq += dir;
            piece = b_[sqq];
        }
    }

    // bishops & queens
    for(auto dir : pieces[wb].move_directions) {
        auto sqq = sq + dir;
        auto piece = b_[sqq];
        while(piece != ESquare::offboard) {
            if(piece != empty) {
                if(pieces[piece].is_bishop_queen && pieces[piece].side == attacking_side)
                    return true;
                break;
            }
            sqq += dir;
            piece = b_[sqq];
        }
    }

    // kings
    for(auto dir : pieces[wk].move_directions) {
        const auto piece = b_[sq + dir];
        if(piece != ESquare::offboard && pieces[piece].is_king && pieces[piece].side == attacking_side)
            return true;
    }

    return false;
}

bool TBoard::is_valid()
{
    int32 piece_count[n_pieces];
    uint32 n_big_pieces[2] = {0, 0};
    uint32 n_major_pieces[2] = {0, 0};
    uint32 n_minor_pieces[2] = {0, 0};
    uint32 material_score[2] = {0, 0};

    TBitboard pawns[3];
    pawns[0] = pawns_[0];
    pawns[1] = pawns_[1];
    pawns[2] = pawns_[2];

    for(auto& pc : piece_count)
        pc = 0;

    for(uint32 p = EPieceType::wp; p <= EPieceType::bk; ++p)
        for(auto sq : piece_locations_[p])
            MAKE_SURE(b_[sq] == p);

    for(uint32 sq64 = 0; sq64 < n_board_squares; sq64++) {
        const auto sq120 = ESquare::Sq120(sq64);
        const auto p = b_[sq120];
        const auto piece = pieces[p];
        piece_count[p]++;

        const auto side = piece.side;
        if(piece.is_big) {
            n_big_pieces[side]++;
            if(piece.is_major)
                n_major_pieces[side]++;
            else if(piece.is_minor)
                n_minor_pieces[side]++;
        }

        material_score[side] += piece.value;
    }

    for(uint32 p = EPieceType::wp; p <= EPieceType::bk; ++p) {
        MAKE_SURE(piece_count[p] == piece_locations_[p].Num());
    }

    MAKE_SURE(pawns[ESide::white].Count() == piece_locations_[EPieceType::wp].Num());
    MAKE_SURE(pawns[ESide::black].Count() == piece_locations_[EPieceType::bp].Num());
    MAKE_SURE(pawns[ESide::both].Count() == piece_locations_[EPieceType::wp].Num() 
		+ piece_locations_[EPieceType::bp].Num());

    while(!pawns[ESide::white].IsEmpty()) {
        const auto sq64 = pawns[ESide::white].Pop();
        MAKE_SURE(b_[ESquare::Sq120(sq64)] == EPieceType::wp);
    }

    while(!pawns[ESide::black].IsEmpty()) {
        const auto sq64 = pawns[ESide::black].Pop();
        MAKE_SURE(b_[ESquare::Sq120(sq64)] == EPieceType::bp);
    }

    while(!pawns[ESide::both].IsEmpty()) {
        const auto sq64 = pawns[ESide::both].Pop();
        MAKE_SURE(b_[ESquare::Sq120(sq64)] == EPieceType::wp
            || b_[ESquare::Sq120(sq64)] == EPieceType::bp);
    }

    MAKE_SURE(material_score[ESide::white] == material_score_[ESide::white]
        && material_score[ESide::black] == material_score_[ESide::black]);
    MAKE_SURE(n_minor_pieces[ESide::white] == n_minor_pieces_[ESide::white]
        && n_minor_pieces[ESide::black] == n_minor_pieces_[ESide::black]);
    MAKE_SURE(n_major_pieces[ESide::white] == n_major_pieces_[ESide::white]
        && n_major_pieces[ESide::black] == n_major_pieces_[ESide::black]);
    MAKE_SURE(n_big_pieces[ESide::white] == n_big_pieces_[ESide::white]
        && n_big_pieces[ESide::black] == n_big_pieces_[ESide::black]);
    MAKE_SURE(side_ == ESide::white || side_ == ESide::black);
    MAKE_SURE(generate_pos_key() == pos_key_);

    MAKE_SURE(en_passant_sq_ == ESquare::no_sq ||
        ESquare::Rank(en_passant_sq_) == ERank::rank_6 && side_ == ESide::white ||
        ESquare::Rank(en_passant_sq_) == ERank::rank_3 && side_ == ESide::black);

    MAKE_SURE(b_[king_sq_[ESide::white]] == EPieceType::wk);
    MAKE_SURE(b_[king_sq_[ESide::black]] == EPieceType::bk);

    return true;
}

FString TBoard::ToString() const
{
    char r[] = ".PNBRQKpnbrqk";
    FString str;
    for(int32 rank = ERank::rank_8; rank >= ERank::rank_1; rank--) {
        for(int32 file = EFile::file_a; file <= EFile::file_h; file++) {
            const auto piece = b_[ESquare::Sq120(file, rank)];
            str += FString::Printf(TEXT("%3c"), r[piece]);
        }
        str += '\n';
    }

    str += "  ";
    for(int32 file = EFile::file_a; file <= EFile::file_h; file++)
        str += "---";
    str += "\n    ";
    return str;
}

void TBoard::perft(const int32 depth, int64* leaf_nodes)
{
    MAKE_SURE(is_valid());

    if(depth == 0) {
        ++*leaf_nodes;
        return;
    }

    auto moves = generate_moves();
    for(auto i = 0; i < moves.Num(); i++) {
        if(!make_move(moves[i]))
            continue;

        perft(depth - 1, leaf_nodes);
        take_move();
    }
}

FString TBoard::perf_test(const int32 depth)
{
    MAKE_SURE(is_valid());
    int64 leaf_nodes = 0;
    auto moves = generate_moves();
    for(auto& m : moves) {
        if(!make_move(m))
            continue;
        perft(depth - 1, &leaf_nodes);
        take_move();
    }
    return FString::Printf(TEXT(" ;D%d %d"), depth, leaf_nodes);
}

int32 TBoard::evaluate()
{
    int32 score = material_score_[ESide::white] - material_score_[ESide::black];

    /*~ white pawn ~*/
    for(auto sq : piece_locations_[EPieceType::wp]) {
        MAKE_SURE(Verification::IsSquareOnBoard(sq));
        score += PawnTable[ESquare::Sq64(sq)];
    }

    /*~ black pawn ~*/
    for(auto sq : piece_locations_[EPieceType::bp]) {
        MAKE_SURE(Verification::IsSquareOnBoard(sq));
        score -= PawnTable[Mirror[ESquare::Sq64(sq)]];
    }

    /*~ white knight ~*/
    
    for(auto sq : piece_locations_[EPieceType::wn]) {
        MAKE_SURE(Verification::IsSquareOnBoard(sq));
        score += KnightTable[ESquare::Sq64(sq)];
    }

    /*~ black knight ~*/
    for(auto sq : piece_locations_[EPieceType::bn]) {
        MAKE_SURE(Verification::IsSquareOnBoard(sq));
        score -= KnightTable[Mirror[ESquare::Sq64(sq)]];
    }

    /*~ white bishop ~*/
    for(auto sq : piece_locations_[EPieceType::wb]) {
        MAKE_SURE(Verification::IsSquareOnBoard(sq));
        score += BishopTable[ESquare::Sq64(sq)];
    }

    /*~ black bishop ~*/
    for(auto sq : piece_locations_[EPieceType::bb]) {
        MAKE_SURE(Verification::IsSquareOnBoard(sq));
        score -= BishopTable[Mirror[ESquare::Sq64(sq)]];
    }

    /*~ white rook ~*/
    for(auto sq : piece_locations_[EPieceType::wr]) {
        MAKE_SURE(Verification::IsSquareOnBoard(sq));
        score += RookTable[ESquare::Sq64(sq)];
    }

    /*~ black rook ~*/
    for(auto sq : piece_locations_[EPieceType::br]) {
        MAKE_SURE(Verification::IsSquareOnBoard(sq));
        score -= RookTable[Mirror[ESquare::Sq64(sq)]];
    }

    return side_ == ESide::white ? score : -score;
}

void TBoard::search(search_info& info)
{
    pv_table_.empty();
    ply_ = 0;

    //~ iterative deepening
    for(auto depth = 1; depth <= info.depth; ++depth) {
        const auto best_score = alpha_beta(-INFINITE, INFINITE, depth, info, true);

        // out of time check

        const auto pvmoves = pv_table_.get_line(depth);
        const auto best_move = pvmoves[0];

        UE_LOG(LogTemp, Log, TEXT("depth %d, score %d, move: %s, nodes %ld"), depth,
            best_score, *best_move.ToString(), info.nodes);

        FString str = "pv";
        for(auto& move : pvmoves) {
            str += " " + move.ToString();
        }

        UE_LOG(LogTemp, Log, TEXT("%s"), *str);
        UE_LOG(LogTemp, Log, TEXT("Ordering %.2f"), info.fh == 0 ? 0 : info.fhf / info.fh);
    }
}

int32 TBoard::alpha_beta(int32 alpha, const int32 beta, const uint32 depth,
                         search_info& info, const bool do_null)
{
    MAKE_SURE(is_valid());

    info.nodes++;

    if(depth == 0) {
        return evaluate();
    }

    if(fifty_move_counter_ >= 100 || has_repetition())
        return 0; // draw

    if(ply_ > max_depth - 1)
        return evaluate();

    uint32 legal = 0;
    const auto old_alpha = alpha;
    auto moves = generate_moves();
    auto best_move = TMove::no_move;

    moves.Sort([](const TMove& lhs, const TMove& rhs) -> bool
    {
        return lhs.score() > rhs.score();
    });

    for(auto& move : moves) {// i = 0; i < moves.Num(); ++i) {
        //auto move = moves[i];
        if(!make_move(move))
            continue;

        legal++;
        const auto score = -alpha_beta(-beta, -alpha, depth - 1, info, do_null);
        take_move();

        if(score > alpha) {
            if(score >= beta) {
                if(legal == 1)
                    info.fhf++;
                info.fh++;

                if(!move.is_captured()) {
                    info.add_killer(ply_, move);
                }

                return beta;
            }
            alpha = score;
            best_move = move;

            if(!move.is_captured()) {
                info.history[b_[best_move.from()]][best_move.to()] += depth;
			}
        }
    }

    if(legal == 0) {
        if(is_attacked(king_sq_[side_], side_ ^ 1))
            return -MATE + ply_; // mate
        return 0; // stalemate and draw
    }

    if(alpha != old_alpha)
        pv_table_.add_move(best_move, pos_key_);

    return alpha;
}

int32 TBoard::quiescence(int32 alpha, int32 beta, search_info& info)
{
    return 0;
}

bool TBoard::has_repetition()
{
    for(auto i = history_.Num() - fifty_move_counter_; i < history_.Num() - 1; ++i) {
        if(history_[i].pos_key == pos_key_)
            return true;
    }
    return false;
}

void TBoard::add_piece(const uint32 sq, const uint32 piece)
{
    MAKE_SURE(Verification::IsSquareOnBoard(sq));
    MAKE_SURE(Verification::IsPieceValid(piece));

    const auto p = pieces[piece];
    HASH_PIECE(piece, sq);
    b_[sq] = piece;

    if(p.is_big) {
        n_big_pieces_[p.side]++;
        if(p.is_major)
            n_major_pieces_[p.side]++;
        else
            n_minor_pieces_[p.side]++;
    } else {
        pawns_[p.side].SetSquare(ESquare::Sq64(sq));
        pawns_[ESide::both].SetSquare(ESquare::Sq64(sq));
    }
    material_score_[p.side] += p.value;
    piece_locations_[piece].Add(sq);
}

void TBoard::move_piece(const uint32 from, const uint32 to)
{
    MAKE_SURE(Verification::IsSquareOnBoard(from));
    MAKE_SURE(Verification::IsSquareOnBoard(to));

    const auto p = b_[from];
    const auto pp = pieces[p];

    HASH_PIECE(p, from);
    b_[from] = EPieceType::empty;
    HASH_PIECE(p, to);
    b_[to] = p;

    if(pp.is_pawn) {
        pawns_[pp.side].ClearSquare(ESquare::Sq64(from));
        pawns_[ESide::both].ClearSquare(ESquare::Sq64(from));
        pawns_[pp.side].SetSquare(ESquare::Sq64(to));
        pawns_[ESide::both].SetSquare(ESquare::Sq64(to));
    }

    for(auto& sq : piece_locations_[p]) {
        if(sq == from) {
            sq = to;
            break;
        }
    }
}

void TBoard::clear_piece(const uint32 sq)
{
    MAKE_SURE(Verification::IsSquareOnBoard(sq));
    MAKE_SURE(is_valid());
    const auto p = b_[sq];
    const auto pp = pieces[p];
    MAKE_SURE(Verification::IsPieceValid(p));

    HASH_PIECE(p, sq);
    b_[sq] = EPieceType::empty;
    material_score_[pp.side] -= pp.value;

    if(pp.is_big) {
        n_big_pieces_[pp.side]--;
        if(pp.is_major)
            n_major_pieces_[pp.side]--;
        else
            n_minor_pieces_[pp.side]--;
    } else {
        pawns_[pp.side].ClearSquare(ESquare::Sq64(sq));
        pawns_[ESide::both].ClearSquare(ESquare::Sq64(sq));
    }

    piece_locations_[p].RemoveSingleSwap(sq);
}
