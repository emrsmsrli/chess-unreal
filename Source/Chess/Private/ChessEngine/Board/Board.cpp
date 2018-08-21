// Copyright 2018 Emre Simsirli

#include "ChessEngine/Board/Board.h"
#include "ChessEngine/Definitions/BoardDefs.h"
#include "ChessEngine/Definitions/PosKey.h"
#include "ChessEngine/Definitions/Piece.h"
#include "ChessEngine/Definitions/Transition.h"
#include "ChessEngine/Definitions/Square.h"
#include "ChessEngine/Undo.h"
#include "ChessEngine/Definitions/Verify.h"
#include "ChessEngine/Definitions/Team.h"

#define MAX_POSITION_MOVES 256

#define HASH_PIECE(p, sq)   (pos_key_ ^= PosKey::GetPieceKey(p, sq))
#define HASH_CASTL()        (pos_key_ ^= PosKey::GetCastleKey(cast_perm_))
#define HASH_SIDE()         (pos_key_ ^= PosKey::GetSideKey())
#define HASH_EN_P()         (pos_key_ ^= PosKey::GetPieceKey(EPieceType::empty, en_passant_sq_))

constexpr auto start_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

namespace {
    const int32 direction_knight[] = {-8, -19, -21, -12, 8, 19, 21, 12};
    const int32 direction_rook[] = {-1, -10, 1, 10};
    const int32 direction_bishop[] = {-9, -11, 9, 11};
    const int32 direction_king[] = {-1, -10, -9, -11, 1, 10, 9, 11};

    const int32 l_sliding_pieces[] = {
        EPieceType::wb, EPieceType::wr, EPieceType::wq, 0,
        EPieceType::bb, EPieceType::br, EPieceType::bq, 0
    };

    const int32 l_non_sliding_pieces[] = {
        EPieceType::wn, EPieceType::wk, 0,
        EPieceType::bn, EPieceType::bk, 0
    };

    const int32 l_slide_index[] = {0, 4};
    const int32 l_non_slide_index[] = {0, 3};

    const int32 piece_directions[][8] = {
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {-8, -19, -21, -12, 8, 19, 21, 12},
        {-9, -11, 9, 11, 0, 0, 0, 0},
        {-1, -10, 1, 10, 0, 0, 0, 0},
        {-1, -10, 1, 10, -9, -11, 9, 11},
        {-1, -10, 1, 10, -9, -11, 9, 11},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {-8, -19, -21, -12, 8, 19, 21, 12},
        {-9, -11, 9, 11, 0, 0, 0, 0},
        {-1, -10, 1, 10, 0, 0, 0, 0},
        {-1, -10, 1, 10, -9, -11, 9, 11},
        {-1, -10, 1, 10, -9, -11, 9, 11},
    };

    const uint32 num_dir[] = {
        0, 0, 8, 4, 4, 8, 8, 0, 8, 4, 4, 8, 8
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

TBoard::TBoard() {
    set(start_fen);
}

void TBoard::reset() {
    for(auto& sq : b_)
        sq = ESquare::offboard;

    for(uint32 sq = 0; sq < n_board_squares; ++sq)
        b_[Transition::sq120(sq)] = EPieceType::empty;

    for(auto& pawns : pawns_)
        pawns.Empty();

    for(uint32 i = 0; i < 2; ++i) {
        n_big_pieces_[i] = 0;
        n_major_pieces_[i] = 0;
        n_minor_pieces_[i] = 0;
        material_score_[i] = 0;
    }

    for(auto& i : piece_count_)
        i = 0;

    for(uint32 side = ETeam::WHITE; side <= ETeam::BLACK; ++side)
        king_sq_[side] = ESquare::no_sq;

    history_.Reset();
    side_ = ETeam::BOTH;
    en_passant_sq_ = ESquare::no_sq;
    fifty_move_counter_ = 0;
    current_search_ply_ = 0;
    cast_perm_ = 0;
    pos_key_ = 0;
}

bool TBoard::set(const FString& fen) {
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
            const auto sq120 = Transition::fr_sq120(file, rank);
            if(piece != EPieceType::empty)
                b_[sq120] = piece;
        }

        file += count;
    }

    ensure(*f == 'w' || *f == 'b');

    side_ = (*f == 'w') ? ETeam::WHITE : ETeam::BLACK;
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

    ensure(cast_perm_ >= 0 && cast_perm_ < 16);
    ensure(*f != ' ');

    if(*f != '-') {
        const uint32 file = f[0] - 'a';
        const uint32 rank = f[1] - '1';

        ensure(file >= EFile::file_a && file <= EFile::file_h);
        ensure(rank >= ERank::rank_1 && rank <= ERank::rank_8);

        en_passant_sq_ = Transition::fr_sq120(file, rank);
    }

    pos_key_ = generate_pos_key();
    update_material();
    return true;
}

uint64 TBoard::generate_pos_key() {
    uint64 key = 0;
    for(uint32 sq = 0; sq < n_board_squares_x; ++sq) {
        const auto p = b_[sq];
        if(p != ESquare::no_sq && p != ESquare::offboard && p != EPieceType::empty) {
            ensure(p >= EPieceType::wp && p <= EPieceType::bk);
            key ^= PosKey::GetPieceKey(p, sq);
        }
    }

    if(side_ == ETeam::WHITE) {
        key ^= PosKey::GetSideKey();
    }

    if(en_passant_sq_ != ESquare::no_sq) {
        ensure(en_passant_sq_ >= 0 && en_passant_sq_ < n_board_squares_x);
        key ^= PosKey::GetPieceKey(EPieceType::empty, en_passant_sq_);
    }

    ensure(cast_perm_ >= 0 && cast_perm_ < 16);
    key ^= PosKey::GetCastleKey(cast_perm_);
    return key;
}

void TBoard::update_material() {
    for(uint32 sq = 0; sq < n_board_squares_x; ++sq) {
        const auto piece = b_[sq];
        if(piece != ESquare::offboard && piece != EPieceType::empty) {
            const auto p = pieces[piece];
            const auto side = p.side;

            if(p.is_big) n_big_pieces_[side]++;
            if(p.is_major) n_major_pieces_[side]++;
            if(p.is_minor) n_minor_pieces_[side]++;

            material_score_[side] += p.value;

            piece_list_[piece][piece_count_[piece]] = sq;
            piece_count_[piece]++;

            switch(piece) {
            case EPieceType::wk:
                king_sq_[ETeam::WHITE] = sq;
                break;
            case EPieceType::bk:
                king_sq_[ETeam::BLACK] = sq;
                break;
            case EPieceType::wp:
                pawns_[ETeam::WHITE].SetSquare(Transition::sq64(sq));
                pawns_[ETeam::BOTH].SetSquare(Transition::sq64(sq));
                break;
            case EPieceType::bp:
                pawns_[ETeam::BLACK].SetSquare(Transition::sq64(sq));
                pawns_[ETeam::BOTH].SetSquare(Transition::sq64(sq));
                break;
            default: break;
            }
        }
    }
}

bool TBoard::is_attacked(const uint32 sq, const uint32 side) {
    ensure(Verification::IsSquareOnBoard(sq));
    ensure(Verification::IsSideValid(side));
    ensure(is_valid());

    // pawns
    if(side == ETeam::WHITE) {
        if(b_[sq - 11] == EPieceType::wp || b_[sq - 9] == EPieceType::wp)
            return true;
    } else {
        if(b_[sq + 11] == EPieceType::bp || b_[sq + 9] == EPieceType::bp)
            return true;
    }

    // knights
    for(auto dir : direction_knight) {
        const auto p = b_[sq + dir];
        const auto piece = pieces[p];
        if(p != ESquare::offboard && piece.is_knight && piece.side == side)
            return true;
    }

    // rooks & queens
    for(auto dir : direction_rook) {
        auto sqq = sq + dir;
        auto piece = b_[sqq];
        while(piece != ESquare::offboard) {
            if(piece != EPieceType::empty) {
                if(pieces[piece].is_rook_queen && pieces[piece].side == side)
                    return true;
                break;
            }
            sqq += dir;
            piece = b_[sqq];
        }
    }

    // bishops & queens
    for(auto dir : direction_bishop) {
        auto sqq = sq + dir;
        auto piece = b_[sqq];
        while(piece != ESquare::offboard) {
            if(piece != EPieceType::empty) {
                if(pieces[piece].is_bishop_queen && pieces[piece].side == side)
                    return true;
                break;
            }
            sqq += dir;
            piece = b_[sqq];
        }
    }

    // kings
    for(auto dir : direction_king) {
        const auto piece = b_[sq + dir];
        if(piece != ESquare::offboard && pieces[piece].is_king && pieces[piece].side == side)
            return true;
    }

    return false;
}


//~ todo refactor with going through all valid pieces
//~ generate_white_moves();
//~ generate_black_moves();
//~ generate_moves(sq);
//~ generate_pawn_moves(sq);
//~ generate_sliding_moves(sq);
TArray<TMove> TBoard::generate_moves() {
    ensure(is_valid());
    TArray<TMove> moves;

    if(side_ == ETeam::WHITE) {
        for(uint32 p_count = 0; p_count < piece_count_[EPieceType::wp]; ++p_count) {
            const auto sq = piece_list_[EPieceType::wp][p_count];
            ensure(Verification::IsSquareOnBoard(sq));

            if(b_[sq + 10] == EPieceType::empty) {
                add_white_pawn_move(sq, sq + 10, moves);
                if(Square::Rank(sq) == ERank::rank_2 && b_[sq + 20] == EPieceType::empty)
                    add_quiet_move(TMove::create(sq, sq + 20, EPieceType::empty,
                                                EPieceType::empty, TMove::flag_pawn_start), moves);
            }

            if(Verification::IsSquareOnBoard(sq + 9) && pieces[b_[sq + 9]].side == ETeam::BLACK)
                add_white_pawn_capture_move(sq, sq + 9, b_[sq + 9], moves);
            if(Verification::IsSquareOnBoard(sq + 11) && pieces[b_[sq + 11]].side == ETeam::BLACK)
                add_white_pawn_capture_move(sq, sq + 11, b_[sq + 11], moves);

            if(sq + 9 == en_passant_sq_)
                add_capture_move(TMove::create(sq, sq + 9, EPieceType::empty, EPieceType::empty,
                                              TMove::flag_en_passant), moves);
            if(sq + 11 == en_passant_sq_)
                add_capture_move(TMove::create(sq, sq + 11, EPieceType::empty, EPieceType::empty,
                                              TMove::flag_en_passant), moves);
        }

        if(cast_perm_ & ECastlingPermission::c_wk) {
            if(b_[ESquare::f1] == EPieceType::empty && b_[ESquare::g1] == EPieceType::empty) {
                if(!is_attacked(ESquare::e1, ETeam::BLACK) && !is_attacked(ESquare::f1, ETeam::BLACK)) {
                    add_quiet_move(TMove::create(ESquare::e1, ESquare::g1, EPieceType::empty, EPieceType::empty,
                                                TMove::flag_castling), moves);
                }
            }
        }

        if(cast_perm_ & ECastlingPermission::c_wq) {
            if(b_[ESquare::d1] == EPieceType::empty && b_[ESquare::c1] == EPieceType::empty && b_[ESquare::b1] ==
                EPieceType::empty) {
                if(!is_attacked(ESquare::e1, ETeam::BLACK) && !is_attacked(ESquare::d1, ETeam::BLACK)) {
                    add_quiet_move(TMove::create(ESquare::e1, ESquare::c1, EPieceType::empty, EPieceType::empty,
                                                TMove::flag_castling), moves);
                }
            }
        }
    } else {
        for(uint32 p_count = 0; p_count < piece_count_[EPieceType::bp]; ++p_count) {
            const auto sq = piece_list_[EPieceType::bp][p_count];
            ensure(Verification::IsSquareOnBoard(sq));

            if(b_[sq - 10] == EPieceType::empty) {
                add_black_pawn_move(sq, sq - 10, moves);
                if(Square::Rank(sq) == ERank::rank_7 && b_[sq - 20] == EPieceType::empty)
                    add_quiet_move(TMove::create(sq, sq - 20, EPieceType::empty,
                                                EPieceType::empty, TMove::flag_pawn_start), moves);
            }

            if(Verification::IsSquareOnBoard(sq - 9) && pieces[b_[sq - 9]].side == ETeam::WHITE)
                add_black_pawn_capture_move(sq, sq - 9, b_[sq - 9], moves);
            if(Verification::IsSquareOnBoard(sq - 11) && pieces[b_[sq - 11]].side == ETeam::WHITE)
                add_black_pawn_capture_move(sq, sq - 11, b_[sq - 11], moves);

            if(sq - 9 == en_passant_sq_)
                add_capture_move(TMove::create(sq, sq - 9, EPieceType::empty, EPieceType::empty,
                                              TMove::flag_en_passant), moves);
            if(sq - 11 == en_passant_sq_)
                add_capture_move(TMove::create(sq, sq - 11, EPieceType::empty, EPieceType::empty,
                                              TMove::flag_en_passant), moves);
        }

        if(cast_perm_ & ECastlingPermission::c_bk) {
            if(b_[ESquare::f8] == EPieceType::empty && b_[ESquare::g8] == EPieceType::empty) {
                if(!is_attacked(ESquare::e8, ETeam::WHITE) && !is_attacked(ESquare::f8, ETeam::WHITE)) {
                    add_quiet_move(TMove::create(ESquare::e8, ESquare::g8, EPieceType::empty, EPieceType::empty,
                                                TMove::flag_castling), moves);
                }
            }
        }

        if(cast_perm_ & ECastlingPermission::c_bq) {
            if(b_[ESquare::d8] == EPieceType::empty && b_[ESquare::c8] == EPieceType::empty && b_[ESquare::b8] ==
                EPieceType::empty) {
                if(!is_attacked(ESquare::e8, ETeam::WHITE) && !is_attacked(ESquare::d8, ETeam::WHITE)) {
                    add_quiet_move(TMove::create(ESquare::e8, ESquare::c8, EPieceType::empty, EPieceType::empty,
                                                TMove::flag_castling), moves);
                }
            }
        }
    }

    for(uint32 s_i = l_slide_index[side_]; ; s_i++) {
        const auto piece = l_sliding_pieces[s_i];
        if(piece == EPieceType::empty)
            break;
        ensure(Verification::IsPieceValid(piece));

        for(uint32 p_n = 0; p_n < piece_count_[piece]; ++p_n) {
            const auto sq = piece_list_[piece][p_n];
            ensure(Verification::IsSquareOnBoard(sq));

            for(uint32 i = 0; i < num_dir[piece]; ++i) {
                const auto dir = piece_directions[piece][i];
                auto sqq = sq + dir;

                while(Verification::IsSquareOnBoard(sqq)) {
                    if(b_[sqq] != EPieceType::empty) {
                        if(pieces[b_[sqq]].side == (side_ ^ 1))
                            add_capture_move(TMove::create(sq, sqq, b_[sqq], EPieceType::empty, 0), moves);
                        break;
                    }
                    add_quiet_move(TMove::create(sq, sqq, EPieceType::empty, EPieceType::empty, 0), moves);
                    sqq += dir;
                }
            }
        }
    }

    for(uint32 s_i = l_non_slide_index[side_]; ; s_i++) {
        const auto piece = l_non_sliding_pieces[s_i];
        if(piece == EPieceType::empty)
            break;
        ensure(Verification::IsPieceValid(piece));

        for(uint32 p_n = 0; p_n < piece_count_[piece]; ++p_n) {
            const auto sq = piece_list_[piece][p_n];
            ensure(Verification::IsSquareOnBoard(sq));

            for(uint32 i = 0; i < num_dir[piece]; ++i) {
                const auto dir = piece_directions[piece][i];
                const auto sqq = sq + dir;

                if(!Verification::IsSquareOnBoard(sqq))
                    continue;

                if(b_[sqq] != EPieceType::empty) {
                    if(pieces[b_[sqq]].side == (side_ ^ 1))
                        add_capture_move(TMove::create(sq, sqq, b_[sqq], EPieceType::empty, 0), moves);
                    continue;
                }
                add_quiet_move(TMove::create(sq, sqq, EPieceType::empty, EPieceType::empty, 0), moves);
            }
        }
    }

    return moves;
}

bool TBoard::make_move(const TMove& m) {
    ensure(is_valid());

    const auto from = m.from();
    const auto to = m.to();
    const auto side = side_;

    ensure(Verification::IsSquareOnBoard(from));
    ensure(Verification::IsSquareOnBoard(to));
    ensure(Verification::IsSideValid(side_));
    ensure(Verification::IsPieceValid(b_[from]));

    auto u = FUndo(m);
    u.pos_key = pos_key_;

    if(m.is_enpassant()) {
        if(side_ == ETeam::WHITE) {
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
        ensure(Verification::IsPieceValid(captured));
        clear_piece(to);
        fifty_move_counter_ = 0;
    }

    current_search_ply_++;
    history_.Add(u);

    if(pieces[b_[from]].is_pawn) {
        fifty_move_counter_ = 0;
        if(m.is_pawnstart()) {
            if(side == ETeam::WHITE) {
                en_passant_sq_ = from + 10;
                ensure(Square::Rank(en_passant_sq_) == ERank::rank_3);
            } else {
                en_passant_sq_ = from - 10;
                ensure(Square::Rank(en_passant_sq_) == ERank::rank_6);
            }
            HASH_EN_P();
        }
    }

    move_piece(from, to);

    const auto promoted = m.promoted_piece();
    if(promoted != EPieceType::empty) {
        ensure(Verification::IsPieceValid(promoted) && !pieces[promoted].is_pawn);
        clear_piece(to);
        add_piece(to, promoted);
    }

    if(pieces[b_[to]].is_king) {
        king_sq_[side_] = to;
    }

    side_ = side_ ^ 1;
    HASH_SIDE();

    ensure(is_valid());

    if(is_attacked(king_sq_[side], side_)) {
        take_move();
        return false;
    }

    return true;
}

void TBoard::take_move() {
    ensure(is_valid());

    auto h = history_.Pop();
    current_search_ply_--;

    const auto from = h.move.from();
    const auto to = h.move.to();
    ensure(Verification::IsSquareOnBoard(from));
    ensure(Verification::IsSquareOnBoard(to));

    if(en_passant_sq_ != ESquare::no_sq)
        HASH_EN_P();
    HASH_CASTL();

    cast_perm_ = h.cast_perm;
    fifty_move_counter_ = h.fifty_move_counter;
    en_passant_sq_ = h.en_passant_sq;

    if(en_passant_sq_ != ESquare::no_sq)
        HASH_EN_P();
    HASH_CASTL();

    side_ = side_ ^ 1;
    HASH_SIDE();

    if(h.move.is_enpassant()) {
        if(side_ == ETeam::WHITE) {
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
            ensure(false);
        }
    }

    move_piece(to, from);

    if(pieces[b_[from]].is_king) {
        king_sq_[side_] = from;
    }

    const auto captured = h.move.captured_piece();
    if(captured != EPieceType::empty) {
        ensure(Verification::IsPieceValid(captured));
        add_piece(to, captured);
    }

    const auto promoted = h.move.promoted_piece();
    if(h.move.is_promoted()) {
        ensure(Verification::IsPieceValid(promoted) && !pieces[promoted].is_pawn);
        clear_piece(from);
        add_piece(from, pieces[promoted].side == ETeam::WHITE ? EPieceType::wp : EPieceType::bp);
    }

    ensure(is_valid());
}

bool TBoard::move_exists(const TMove& m) {
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

void TBoard::add_pv_move(const TMove& m) {
	pv_table_.Add(pos_key_, m);
}

TMove TBoard::probe_pv_table() {
    const auto search = pv_table_.Find(pos_key_);
    if(!search)
        return TMove::no_move;
    return *search;
}

uint32 TBoard::get_pv_line(const uint32 depth) {
    ensure(depth < max_depth);

    uint32 count = 0;
    auto m = probe_pv_table();
    while(m != TMove::no_move && count < depth) {
        if(move_exists(m)) {
            make_move(m);
            pv_array_[count++] = m;
        } else break;
        m = probe_pv_table();
    }

    while(current_search_ply_ > 0)
        take_move();
    return count;
}

bool TBoard::is_valid() {
    uint32 piece_count[n_pieces];
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

    for(uint32 p = EPieceType::wp; p <= EPieceType::bk; ++p) {
        for(uint32 n_p = 0; n_p < piece_count_[p]; ++n_p) {
            const auto sq120 = piece_list_[p][n_p];
            ensure(b_[sq120] == p);
        }
    }

    for(uint32 sq64 = 0; sq64 < n_board_squares; sq64++) {
        const auto sq120 = Transition::sq120(sq64);
        const auto p = b_[sq120];
        piece_count[p]++;
        
    // todo pawns[0] changes here?
        const auto side = pieces[p].side;
        if(pieces[p].is_big) n_big_pieces[side]++;
        if(pieces[p].is_major) n_major_pieces[side]++;
        if(pieces[p].is_minor) n_minor_pieces[side]++;

        material_score[side] += pieces[p].value;
    }

    for(uint32 p = EPieceType::wp; p <= EPieceType::bk; ++p) {
        ensure(piece_count[p] == piece_count_[p]);
    }

    ensure(pawns[ETeam::WHITE].Count() == piece_count_[EPieceType::wp]);
    ensure(pawns[ETeam::BLACK].Count() == piece_count_[EPieceType::bp]);
    ensure(pawns[ETeam::BOTH].Count() == piece_count_[EPieceType::wp] + piece_count_[EPieceType::bp]);

    while(!pawns[ETeam::WHITE].IsEmpty()) {
        const auto sq64 = pawns[ETeam::WHITE].Pop();
        ensure(b_[Transition::sq120(sq64)] == EPieceType::wp);
    }

    while(!pawns[ETeam::BLACK].IsEmpty()) {
        const auto sq64 = pawns[ETeam::BLACK].Pop();
        ensure(b_[Transition::sq120(sq64)] == EPieceType::bp);
    }

    while(!pawns[ETeam::BOTH].IsEmpty()) {
        const auto sq64 = pawns[ETeam::BOTH].Pop();
        ensure(b_[Transition::sq120(sq64)] == EPieceType::wp || b_[Transition::sq120(sq64)] == EPieceType::bp);
    }

    ensure(material_score[ETeam::WHITE] == material_score_[ETeam::WHITE]
        && material_score[ETeam::BLACK] == material_score_[ETeam::BLACK]);
    ensure(n_minor_pieces[ETeam::WHITE] == n_minor_pieces_[ETeam::WHITE]
        && n_minor_pieces[ETeam::BLACK] == n_minor_pieces_[ETeam::BLACK]);
    ensure(n_major_pieces[ETeam::WHITE] == n_major_pieces_[ETeam::WHITE]
        && n_major_pieces[ETeam::BLACK] == n_major_pieces_[ETeam::BLACK]);
    ensure(n_big_pieces[ETeam::WHITE] == n_big_pieces_[ETeam::WHITE]
        && n_big_pieces[ETeam::BLACK] == n_big_pieces_[ETeam::BLACK]);
    ensure(side_ == ETeam::WHITE || side_ == ETeam::BLACK);
    ensure(generate_pos_key() == pos_key_);

    ensure(en_passant_sq_ == ESquare::no_sq ||
        Square::Rank(en_passant_sq_) == ERank::rank_6 && side_ == ETeam::WHITE ||
        Square::Rank(en_passant_sq_) == ERank::rank_3 && side_ == ETeam::BLACK);

    ensure(b_[king_sq_[ETeam::WHITE]] == EPieceType::wk);
    ensure(b_[king_sq_[ETeam::BLACK]] == EPieceType::bk);

    return true;
}

FString TBoard::ToString() const {
    FString str;

    str += pawns_[0].ToString() + '\n';
    str += pawns_[1].ToString() + '\n';
    str += pawns_[2].ToString() + '\n';

    for(int32 rank = ERank::rank_8; rank >= ERank::rank_1; rank--) {
        //str += (char)(rank - 1 + 'A') + " | ";
        for(int32 file = EFile::file_a; file <= EFile::file_h; file++) {
            const auto piece = b_[Transition::fr_sq120(file, rank)];
            str += FString::Printf(TEXT("%3d"), piece);
            //stream << std::setw(3) << std::left << representation::pieces[piece];
        }
        str += '\n';
    }

    str += "  ";
    for(int32 file = EFile::file_a; file <= EFile::file_h; file++)
        str += "---";
    str += "\n    ";
    /*for(int32 file = EFile::file_a; file <= EFile::file_h; file++)
        stream << std::setw(3) << std::left << representation::files[file];*/

    /*str << "\nside: " << representation::sides[side_] << "\nen pas sq: " << en_passant_sq_;
    str << "\ncastling: "
        << (cast_perm_ & castling_permissions::c_wk ? 'K' : '-')
        << (cast_perm_ & castling_permissions::c_wq ? 'Q' : '-')
        << (cast_perm_ & castling_permissions::c_bk ? 'k' : '-')
        << (cast_perm_ & castling_permissions::c_bq ? 'q' : '-');
    stream << "\npos key: " << std::hex << pos_key_;*/

    // todo remove this -- stream << "\nwp:\n" << pawns_[side::white].str() << "\nbp:\n" << pawns_[side::black].str() << '\n';
    return str;
}

void TBoard::perft(const int32 depth, int64* leaf_nodes) {
    ensure(is_valid());

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

FString TBoard::perf_test(const int32 depth) {
    FString str;
    ensure(is_valid());
    str += "starting perft, depth: " + FString::FromInt(depth) + '\n';
    int64 leaf_nodes = 0;
    auto moves = generate_moves();
    for(auto& m : moves) {
        if(!make_move(m))
            continue;
        const auto cumumlative_nodes = leaf_nodes;
        perft(depth - 1, &leaf_nodes);
        take_move();

        const auto old_nodes = leaf_nodes - cumumlative_nodes;
        str += FString::Printf(TEXT("move: %d-%d : %ld\n"), m.from(), m.to(), old_nodes);
    }
    str += "test complete: " + FString::FromInt(leaf_nodes) + " visited";
    return str;
}

bool TBoard::has_repetition() {
	for(auto i = 0; i < history_.Num(); i++) {
        if(history_[i].pos_key == pos_key_)
            return true;
	}
    return false;
}

void TBoard::add_piece(uint32 sq, uint32 piece) {
    ensure(Verification::IsSquareOnBoard(sq));
    ensure(Verification::IsPieceValid(piece));

    const auto p = pieces[piece];
    HASH_PIECE(piece, sq);
    b_[sq] = piece;

    if(p.is_big) {
        n_big_pieces_[p.side]++;
        if(p.is_major) {
            n_major_pieces_[p.side]++;
        } else {
            n_minor_pieces_[p.side]++;
        }
    } else {
        pawns_[p.side].SetSquare(Transition::sq64(sq));
        pawns_[ETeam::BOTH].SetSquare(Transition::sq64(sq));
    }
    material_score_[p.side] += p.value;
    piece_list_[piece][piece_count_[piece]++] = sq;
}

void TBoard::move_piece(uint32 from, uint32 to) {
    ensure(Verification::IsSquareOnBoard(from));
    ensure(Verification::IsSquareOnBoard(to));

    const auto p = b_[from];
    const auto pp = pieces[p];

    HASH_PIECE(p, from);
    b_[from] = EPieceType::empty;
    HASH_PIECE(p, to);
    b_[to] = p;

    if(!pp.is_big) {
        pawns_[pp.side].ClearSquare(Transition::sq64(from));
        pawns_[ETeam::BOTH].ClearSquare(Transition::sq64(from));
        pawns_[pp.side].SetSquare(Transition::sq64(to));
        pawns_[ETeam::BOTH].SetSquare(Transition::sq64(to));
    }

    for(uint32 i = 0; i < piece_count_[p]; ++i) {
        if(piece_list_[p][i] == from) {
            piece_list_[p][i] = to;
            break;
        }
    }
}

void TBoard::clear_piece(uint32 sq) {
    ensure(Verification::IsSquareOnBoard(sq));
    ensure(is_valid());
    const auto p = b_[sq];
    const auto pp = pieces[p];
    ensure(Verification::IsPieceValid(p));

    HASH_PIECE(p, sq);
    b_[sq] = EPieceType::empty;
    material_score_[pp.side] -= pp.value;

    if(pp.is_big) {
        n_big_pieces_[pp.side]--;
        if(pp.is_major) {
            n_major_pieces_[pp.side]--;
        } else {
            n_minor_pieces_[pp.side]--;
        }
    } else {
        pawns_[pp.side].ClearSquare(Transition::sq64(sq));
        pawns_[ETeam::BOTH].ClearSquare(Transition::sq64(sq));
    }

    auto t_p = -1;
    for(uint32 i = 0; i < piece_count_[p]; ++i) {
        if(piece_list_[p][i] == sq) {
            t_p = i;
            break;
        }
    }
    ensure(t_p != -1);
    piece_list_[p][t_p] = piece_list_[p][--piece_count_[p]];
}

void TBoard::add_white_pawn_capture_move(uint32 from, uint32 to, 
										 uint32 captured, TArray<TMove>& moves) {
    ensure(Verification::IsPieceValidOrEmpty(captured));
    ensure(Verification::IsSquareOnBoard(from));
    ensure(Verification::IsSquareOnBoard(to));

    if(Square::Rank(from) == ERank::rank_7) {
        add_capture_move(TMove::create(from, to, captured, EPieceType::wq, 0), moves);
        add_capture_move(TMove::create(from, to, captured, EPieceType::wr, 0), moves);
        add_capture_move(TMove::create(from, to, captured, EPieceType::wb, 0), moves);
        add_capture_move(TMove::create(from, to, captured, EPieceType::wn, 0), moves);
    } else {
        add_capture_move(TMove::create(from, to, captured, EPieceType::empty, 0), moves);
    }
}

void TBoard::add_white_pawn_move(uint32 from, uint32 to, TArray<TMove>& moves) {
    ensure(Verification::IsSquareOnBoard(from));
    ensure(Verification::IsSquareOnBoard(to));

    if(Square::Rank(from) == ERank::rank_7) {
        add_quiet_move(TMove::create(from, to, EPieceType::empty, EPieceType::wq, 0), moves);
        add_quiet_move(TMove::create(from, to, EPieceType::empty, EPieceType::wr, 0), moves);
        add_quiet_move(TMove::create(from, to, EPieceType::empty, EPieceType::wb, 0), moves);
        add_quiet_move(TMove::create(from, to, EPieceType::empty, EPieceType::wn, 0), moves);
    } else {
        add_quiet_move(TMove::create(from, to, EPieceType::empty, EPieceType::empty, 0), moves);
    }
}

void TBoard::add_black_pawn_capture_move(uint32 from, uint32 to, uint32 captured, TArray<TMove>& moves) {
    ensure(Verification::IsPieceValidOrEmpty(captured));
    ensure(Verification::IsSquareOnBoard(from));
    ensure(Verification::IsSquareOnBoard(to));

    if(Square::Rank(from) == ERank::rank_2) {
        add_capture_move(TMove::create(from, to, captured, EPieceType::bq, 0), moves);
        add_capture_move(TMove::create(from, to, captured, EPieceType::br, 0), moves);
        add_capture_move(TMove::create(from, to, captured, EPieceType::bb, 0), moves);
        add_capture_move(TMove::create(from, to, captured, EPieceType::bn, 0), moves);
    } else {
        add_capture_move(TMove::create(from, to, captured, EPieceType::empty, 0), moves);
    }
}

void TBoard::add_black_pawn_move(uint32 from, uint32 to, TArray<TMove>& moves) {
    ensure(Verification::IsSquareOnBoard(from));
    ensure(Verification::IsSquareOnBoard(to));

    if(Square::Rank(from) == ERank::rank_2) {
        add_quiet_move(TMove::create(from, to, EPieceType::empty, EPieceType::bq, 0), moves);
        add_quiet_move(TMove::create(from, to, EPieceType::empty, EPieceType::br, 0), moves);
        add_quiet_move(TMove::create(from, to, EPieceType::empty, EPieceType::bb, 0), moves);
        add_quiet_move(TMove::create(from, to, EPieceType::empty, EPieceType::bn, 0), moves);
    } else {
        add_quiet_move(TMove::create(from, to, EPieceType::empty, EPieceType::empty, 0), moves);
    }
}

void TBoard::add_quiet_move(TMove move, TArray<TMove>& moves) {
    moves.Add(move);
}

void TBoard::add_capture_move(TMove move, TArray<TMove>& moves) {
    moves.Add(move);
}

void TBoard::add_en_passant_move(TMove move, TArray<TMove>& moves) {
    moves.Add(move);
}
