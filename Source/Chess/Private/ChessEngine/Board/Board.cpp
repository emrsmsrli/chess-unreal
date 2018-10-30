// Copyright 2018 Emre Simsirli

#include "Board.h"
#include "Search.h"
#include "PosKey.h"
#include "PieceInfo.h"
#include "Square.h"
#include "Undo.h"
#include "Verify.h"
#include "Side.h"
#include "Util/Log.h"

#define MAX_POSITION_MOVES 256

#define HASH_PIECE(p, sq)   (pos_key_ ^= PosKey::GetPieceKey(p, sq))
#define HASH_CASTL()        (pos_key_ ^= PosKey::GetCastleKey(cast_perm_))
#define HASH_SIDE()         (pos_key_ ^= PosKey::GetSideKey())
#define HASH_EN_P()         (pos_key_ ^= PosKey::GetPieceKey(EPieceType::empty, en_passant_sq_))

namespace
{
    constexpr auto start_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

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

UBoard::UBoard()
{
    Set(start_fen);
}

void UBoard::Reset()
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

bool UBoard::Set(const FString& fen)
{
    Reset();

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
                LOGW("deformed char %c in fen: %s", *f, *fen);
                return false;
        }

        for(uint32 i = 0; i < count; ++i) {
            const auto sq120 = ESquare::Sq120(file, rank);
            if(piece != EPieceType::empty)
                b_[sq120] = piece;
            file++;
        }
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

    pos_key_ = GeneratePositionKey();
    UpdateMaterial();

    LOGI("new fen: %s\n%s", *fen, *ToString());

    return true;
}

uint8 UBoard::GetSide() const
{
    return side_;
}

TArray<uint32>* UBoard::GetPieceLocations()
{
    return piece_locations_;
}

uint64 UBoard::GeneratePositionKey() const
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

void UBoard::UpdateMaterial()
{
    for(uint32 sq = 0; sq < n_board_squares_x; ++sq) {
        const auto piece = b_[sq];
        if(piece != ESquare::offboard && piece != EPieceType::empty) {
            const auto piece_info = piece_infos[piece];
            const auto side = piece_info.Side;

            if(piece_info.bIsBig) {
                n_big_pieces_[side]++;
                if(piece_info.bIsMajor)
                    n_major_pieces_[side]++;
                else if(piece_info.bIsMinor)
                    n_minor_pieces_[side]++;
            }

            material_score_[side] += piece_info.Value;
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

bool UBoard::MakeMove(const FMove& m)
{
    MAKE_SURE(IsOk());

    const auto from = m.From();
    const auto to = m.To();
    const auto side = side_;

    MAKE_SURE(Verification::IsSquareOnBoard(from));
    MAKE_SURE(Verification::IsSquareOnBoard(to));
    MAKE_SURE(Verification::IsSideValid(side));
    MAKE_SURE(Verification::IsPieceValid(b_[from]));

    auto u = FUndo(m);
    u.pos_key = pos_key_;

    if(m.IsEnPassant()) {
        if(side == ESide::white) {
            ClearPiece(to - 10);
        } else {
            ClearPiece(to + 10);
        }
    } else if(m.IsCastling()) {
        switch(to) {
            case ESquare::c1:
                MovePiece(ESquare::a1, ESquare::d1);
                break;
            case ESquare::c8:
                MovePiece(ESquare::a8, ESquare::d8);
                break;
            case ESquare::g1:
                MovePiece(ESquare::h1, ESquare::f1);
                break;
            case ESquare::g8:
                MovePiece(ESquare::h8, ESquare::f8);
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

    const auto captured = m.CapturedPiece();
    if(captured != EPieceType::empty) {
        MAKE_SURE(Verification::IsPieceValid(captured));
        ClearPiece(to);
        fifty_move_counter_ = 0;
    }

    ply_++;
    history_.Add(u);

    if(piece_infos[b_[from]].bIsPawn) {
        fifty_move_counter_ = 0;
        if(m.IsPawnStart()) {
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

    MovePiece(from, to);

    const auto promoted = m.PromotedPiece();
    if(promoted != EPieceType::empty) {
        MAKE_SURE(Verification::IsPieceValid(promoted) && !piece_infos[promoted].bIsPawn);
        ClearPiece(to);
        AddPiece(to, promoted);
    }

    if(piece_infos[b_[to]].bIsKing) {
        king_sq_[side] = to;
    }

    side_ ^= 1;
    HASH_SIDE();

    MAKE_SURE(IsOk());

    if(IsAttacked(king_sq_[side], side_)) {
        TakeMove();
        return false;
    }

    return true;
}

void UBoard::TakeMove()
{
    MAKE_SURE(IsOk());

    auto h = history_.Pop();
    ply_--;

    const auto from = h.move.From();
    const auto to = h.move.To();
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

    if(h.move.IsEnPassant()) {
        if(side_ == ESide::white) {
            AddPiece(to - 10, EPieceType::bp);
        } else {
            AddPiece(to + 10, EPieceType::wp);
        }
    } else if(h.move.IsCastling()) {
        switch(to) {
            case ESquare::c1: MovePiece(ESquare::d1, ESquare::a1);
                break;
            case ESquare::c8: MovePiece(ESquare::d8, ESquare::a8);
                break;
            case ESquare::g1: MovePiece(ESquare::f1, ESquare::h1);
                break;
            case ESquare::g8: MovePiece(ESquare::f8, ESquare::h8);
                break;
            default:
            verify(false);
        }
    }

    MovePiece(to, from);

    if(piece_infos[b_[from]].bIsKing) {
        king_sq_[side_] = from;
    }

    const auto captured = h.move.CapturedPiece();
    if(captured != EPieceType::empty) {
        MAKE_SURE(Verification::IsPieceValid(captured));
        AddPiece(to, captured);
    }

    const auto promoted = h.move.PromotedPiece();
    if(h.move.IsPromoted()) {
        MAKE_SURE(Verification::IsPieceValid(promoted) && !piece_infos[promoted].bIsPawn);
        ClearPiece(from);
        AddPiece(from, piece_infos[promoted].Side == ESide::white ? EPieceType::wp : EPieceType::bp);
    }

    MAKE_SURE(IsOk());
}

bool UBoard::IsInCheck()
{
    return IsAttacked(king_sq_[side_], side_ ^ 1);
}

bool UBoard::IsAttacked(const uint32 sq, const uint8 attacking_side) const
{
    MAKE_SURE(Verification::IsSquareOnBoard(sq));
    MAKE_SURE(Verification::IsSideValid(attacking_side));
    MAKE_SURE(IsOk());

    // pawns
    if(attacking_side == ESide::white) {
        if(b_[sq - 11] == wp || b_[sq - 9] == wp)
            return true;
    } else {
        if(b_[sq + 11] == bp || b_[sq + 9] == bp)
            return true;
    }

    // knights
    for(auto dir : piece_infos[wn].MoveDirections) {
        const auto p = b_[sq + dir];
        if(p != ESquare::offboard && piece_infos[p].bIsKnight && piece_infos[p].Side == attacking_side)
            return true;
    }

    // rooks & queens
    for(auto dir : piece_infos[wr].MoveDirections) {
        auto sqq = sq + dir;
        auto piece = b_[sqq];
        while(piece != ESquare::offboard) {
            if(piece != empty) {
                if(piece_infos[piece].bIsRookOrQueen && piece_infos[piece].Side == attacking_side)
                    return true;
                break;
            }
            sqq += dir;
            piece = b_[sqq];
        }
    }

    // bishops & queens
    for(auto dir : piece_infos[wb].MoveDirections) {
        auto sqq = sq + dir;
        auto piece = b_[sqq];
        while(piece != ESquare::offboard) {
            if(piece != empty) {
                if(piece_infos[piece].bIsBishopOrQueen && piece_infos[piece].Side == attacking_side)
                    return true;
                break;
            }
            sqq += dir;
            piece = b_[sqq];
        }
    }

    // kings
    for(auto dir : piece_infos[wk].MoveDirections) {
        const auto piece = b_[sq + dir];
        if(piece != ESquare::offboard && piece_infos[piece].bIsKing && piece_infos[piece].Side == attacking_side)
            return true;
    }

    return false;
}

bool UBoard::HasRepetition()
{
    for(auto i = history_.Num() - fifty_move_counter_; i < history_.Num() - 1; ++i) {
        if(history_[i].pos_key == pos_key_)
            return true;
    }
    return false;
}

bool UBoard::IsDrawByMaterial() const
{
    if(piece_locations_[EPieceType::wp].Num() != 0 ||
        piece_locations_[EPieceType::bp].Num() != 0)
        return false;
    if(piece_locations_[EPieceType::wq].Num() != 0 ||
        piece_locations_[EPieceType::bq].Num() != 0 ||
        piece_locations_[EPieceType::wr].Num() != 0 ||
        piece_locations_[EPieceType::br].Num() != 0)
        return false;
    if(piece_locations_[EPieceType::wb].Num() > 1 ||
        piece_locations_[EPieceType::bb].Num() > 1)
        return false;
    if(piece_locations_[EPieceType::wn].Num() > 1 ||
        piece_locations_[EPieceType::bn].Num() > 1)
        return false;
    if(piece_locations_[EPieceType::wn].Num() != 0 &&
        piece_locations_[EPieceType::wb].Num() != 0)
        return false;
    if(piece_locations_[EPieceType::bn].Num() != 0 &&
        piece_locations_[EPieceType::bb].Num() != 0)
        return false;
    return true;
}

bool UBoard::HasTrifoldRepetition() const
{
    auto rep_count = 0;
    for(auto& h : history_) {
        if(h.pos_key == pos_key_)
            rep_count++;
    }
    return rep_count >= 2;
}

bool UBoard::DoesViolateFiftyMoveRule() const
{
    return fifty_move_counter_ > 100;
}

void UBoard::AddPiece(const uint32 sq, const uint32 piece)
{
    MAKE_SURE(Verification::IsSquareOnBoard(sq));
    MAKE_SURE(Verification::IsPieceValid(piece));

    const auto piece_info = piece_infos[piece];
    HASH_PIECE(piece, sq);
    b_[sq] = piece;

    if(piece_info.bIsBig) {
        n_big_pieces_[piece_info.Side]++;
        if(piece_info.bIsMajor)
            n_major_pieces_[piece_info.Side]++;
        else
            n_minor_pieces_[piece_info.Side]++;
    } else {
        pawns_[piece_info.Side].SetSquare(ESquare::Sq64(sq));
        pawns_[ESide::both].SetSquare(ESquare::Sq64(sq));
    }
    material_score_[piece_info.Side] += piece_info.Value;
    piece_locations_[piece].Add(sq);
}

void UBoard::MovePiece(const uint32 from, const uint32 to)
{
    MAKE_SURE(Verification::IsSquareOnBoard(from));
    MAKE_SURE(Verification::IsSquareOnBoard(to));

    const auto piece = b_[from];
    const auto piece_info = piece_infos[piece];

    HASH_PIECE(piece, from);
    b_[from] = EPieceType::empty;
    HASH_PIECE(piece, to);
    b_[to] = piece;

    if(piece_info.bIsPawn) {
        pawns_[piece_info.Side].ClearSquare(ESquare::Sq64(from));
        pawns_[ESide::both].ClearSquare(ESquare::Sq64(from));
        pawns_[piece_info.Side].SetSquare(ESquare::Sq64(to));
        pawns_[ESide::both].SetSquare(ESquare::Sq64(to));
    }

    for(auto& sq : piece_locations_[piece]) {
        if(sq == from) {
            sq = to;
            break;
        }
    }
}

void UBoard::ClearPiece(const uint32 sq)
{
    MAKE_SURE(Verification::IsSquareOnBoard(sq));
    MAKE_SURE(IsOk());
    const auto piece = b_[sq];
    const auto piece_info = piece_infos[piece];
    MAKE_SURE(Verification::IsPieceValid(piece));

    HASH_PIECE(piece, sq);
    b_[sq] = EPieceType::empty;
    material_score_[piece_info.Side] -= piece_info.Value;

    if(piece_info.bIsBig) {
        n_big_pieces_[piece_info.Side]--;
        if(piece_info.bIsMajor)
            n_major_pieces_[piece_info.Side]--;
        else
            n_minor_pieces_[piece_info.Side]--;
    } else {
        pawns_[piece_info.Side].ClearSquare(ESquare::Sq64(sq));
        pawns_[ESide::both].ClearSquare(ESquare::Sq64(sq));
    }

    piece_locations_[piece].RemoveSingleSwap(sq);
}

#ifdef DEBUG
FString UBoard::ToString() const
{
    char r[] = ".PNBRQKpnbrqk";

    FString str = "  ";
    for(auto file : EFile::All)
        str += "---";

    for(auto rank : ERank::AllReversed) {
        for(auto file : EFile::All) {
            const auto piece = b_[ESquare::Sq120(file, rank)];
            str += FString::Printf(TEXT("%3c"), r[piece]);
        }
        str += '\n';
    }

    str += "  ";
    for(auto file : EFile::All)
        str += "---";

    return str;
}

bool UBoard::IsOk() const
{
    int32 piece_count[n_pieces];
    uint32 n_big_pieces[2] = {0, 0};
    uint32 n_major_pieces[2] = {0, 0};
    uint32 n_minor_pieces[2] = {0, 0};
    uint32 material_score[2] = {0, 0};

    FBitboard pawns[3];
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
        const auto piece = b_[sq120];
        const auto piece_info = piece_infos[piece];
        piece_count[piece]++;

        const auto side = piece_info.Side;
        if(piece_info.bIsBig) {
            n_big_pieces[side]++;
            if(piece_info.bIsMajor)
                n_major_pieces[side]++;
            else if(piece_info.bIsMinor)
                n_minor_pieces[side]++;
        }

        material_score[side] += piece_info.Value;
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
    MAKE_SURE(GeneratePositionKey() == pos_key_);

    MAKE_SURE(en_passant_sq_ == ESquare::no_sq ||
        ESquare::Rank(en_passant_sq_) == ERank::rank_6 && side_ == ESide::white ||
        ESquare::Rank(en_passant_sq_) == ERank::rank_3 && side_ == ESide::black);

    MAKE_SURE(b_[king_sq_[ESide::white]] == EPieceType::wk);
    MAKE_SURE(b_[king_sq_[ESide::black]] == EPieceType::bk);

    return true;
}
#endif
