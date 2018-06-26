#include "Board.h"
#include "ChessEngine.h"
#include "Transition.h"
#include "PosKey.h"

engine::board::board() {
    set(engine::start_fen());
}

void engine::board::reset() {
    for(uint32 sq = 0; sq < N_BOARD_SQUARES_X; ++sq) {
        b_[sq] = square::offboard;
    }

    for(uint32 sq = 0; sq < N_BOARD_SQUARES; ++sq) {
        b_[transition::sq120(sq)] = piece_types::empty;
    }

    for(uint32 i = 0; i < 3; ++i) {
        n_big_pieces_[i] = 0;
        n_major_pieces_[i] = 0;
        n_minor_pieces_[i] = 0;
    }

    for(uint32 i = 0; i < N_PIECES; ++i) {
        piece_count_[i] = 0;
    }

    for(uint32 side = side::white; side <= side::black; ++side)
        king_sq_[side] = square::no_sq;

    side_ = side::both;
    en_passant_sq_ = square::no_sq;
    fifty_move_counter_ = 0;
    current_search_ply_ = 0;
    cast_perm_ = 0;
    pos_key_ = 0;
}

bool engine::board::set(const std::string &fen) {
    reset();
    
    auto f = fen.c_str();
    for(uint32 rank = rank::rank_8, file = file::file_a; rank >= rank::rank_1 && *f; f++) {
        uint32 count = 1;
        auto piece = piece_types::empty;
        switch(*f) {
			case 'p': piece = piece_types::bp; break;
			case 'r': piece = piece_types::br; break;
			case 'n': piece = piece_types::bn; break;
			case 'b': piece = piece_types::bb; break;
			case 'q': piece = piece_types::bq; break;
			case 'k': piece = piece_types::bk; break;
			case 'P': piece = piece_types::wp; break;
			case 'R': piece = piece_types::wr; break;
			case 'N': piece = piece_types::wn; break;
			case 'B': piece = piece_types::wb; break;
			case 'Q': piece = piece_types::wq; break;
			case 'K': piece = piece_types::wk; break;

			case '1': case '2': case '3':
			case '4': case '5': case '6':
			case '7': case '8': case '9': {
                count = *f - '0';
                break;
			}

			case '/': case ' ': {
                rank--;
                file = file::file_a;
                f++;
                continue;
			}

			default:
			    return false;
        }

        for(uint32 i = 0; i < count; ++i) {
            // todo if bugged use auto sq64 = rank * 8 + file; auto sq120 = transition::sq120(sq64);
            const auto sq120 = transition::fr_sq120(file, rank);;
            if(piece != piece_types::empty)
                b_[sq120] = piece;

            file++;
        }
    }

    check(*f  == 'w' || *f == 'b');

    side_ = (*f == 'w') ? side::white : side::black;
    f += 2;

    for(uint32 i = 0; i < 4; ++i) {
        if(*f == ' ')
            break;

        switch(*f) {
			case 'K': cast_perm_ |= castling_permissions::c_wk; break;
			case 'Q': cast_perm_ |= castling_permissions::c_wq; break;
			case 'k': cast_perm_ |= castling_permissions::c_bk; break;
			case 'q': cast_perm_ |= castling_permissions::c_bq; break;
			default:
			    return false;
        }
        f++;
    }
    f++;

    check(cast_perm_ >= 0 && cast_perm_ < 16);
    check(*f != ' ');

    if(*f != '-') {
        const uint32 file = f[0] - 'a';
        const uint32 rank = f[1] - '1';

        check(file >= file::file_a && file <= file::file_h);
        check(rank >= rank::rank_1 && rank <= rank::rank_8);

        en_passant_sq_ = static_cast<square>(transition::fr_sq120(file, rank));
    }

    pos_key_ = generate_pos_key();
    return true;
}

uint64 engine::board::generate_pos_key() {
    uint64 key = 0;
    for(uint32 sq = 0; sq < N_BOARD_SQUARES_X; ++sq) {
        const auto p = b_[sq];
        // bug p can be square::offboard?
        if(p != square::no_sq && p != piece_types::empty) {
            check(p >= piece_types::wp && p <= piece_types::bk);
            key ^= poskey::piece_keys[p][sq];
        }
    }

    if(side_ == side::white) {
        key ^= poskey::side_key;
    }

    if(en_passant_sq_ != square::no_sq) {
        check(en_passant_sq_ >= 0 && en_passant_sq_ < N_BOARD_SQUARES_X);
        key ^= poskey::piece_keys[piece_types::empty][en_passant_sq_];
    }

    check(cast_perm_ >= 0 && cast_perm_ < 16);
    key ^= poskey::castle_keys[cast_perm_];
    return key;
}

engine::square engine::board::king_of(const side side) {
    return king_sq_[side];
}
