#include "Board.h"
#include "ChessEngine.h"
#include "Transition.h"
#include "PosKey.h"
#include "Defs.h"
#include <sstream>
#include <iomanip>

engine::board::board() {
    set(engine::start_fen());
}

void engine::board::reset() {
    for(uint32 sq = 0; sq < N_BOARD_SQUARES_X; ++sq) {
        b_[sq] = square::offboard;
    }

    for(uint32 sq = 0; sq < N_BOARD_SQUARES; ++sq) {
        b_[transition::sq120(sq)] = piece_type::empty;
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

bool engine::board::set(const std::string& fen) {
    reset();

    auto f = fen.c_str();
    for(int32 rank = rank::rank_8, file = file::file_a; rank >= rank::rank_1 && *f; f++) {
        uint32 count = 1;
        auto piece = piece_type::empty;
        switch(*f) {
        case 'p': 
            piece = piece_type::bp;
            break;
        case 'r': 
            piece = piece_type::br;
            break;
        case 'n': 
            piece = piece_type::bn;
            break;
        case 'b': 
            piece = piece_type::bb;
            break;
        case 'q': 
            piece = piece_type::bq;
            break;
        case 'k': 
            piece = piece_type::bk;
            break;
        case 'P': 
            piece = piece_type::wp;
            break;
        case 'R': 
            piece = piece_type::wr;
            break;
        case 'N': 
            piece = piece_type::wn;
            break;
        case 'B': 
            piece = piece_type::wb;
            break;
        case 'Q': 
            piece = piece_type::wq;
            break;
        case 'K': 
            piece = piece_type::wk;
            break;

        case '1': case '2': case '3':
        case '4': case '5': case '6':
        case '7': case '8': {
            count = *f - '0';
            break;
        }

        case '/': case ' ': {
            rank--;
            file = file::file_a;
            continue;
        }

        default:
            return false;
        }

        for(uint32 i = 0; i < count; ++i) {
            const auto sq120 = transition::fr_sq120(file, rank);
            if(piece != piece_type::empty)
                b_[sq120] = piece;
        }

        file+= count;
    }

    ensure(*f == 'w' || *f == 'b');

    side_ = (*f == 'w') ? side::white : side::black;
    f += 2;

    for(uint32 i = 0; i < 4; ++i) {
        if(*f == ' ')
            break;

        switch(*f) {
        case 'K': 
            cast_perm_ |= castling_permissions::c_wk;
            break;
        case 'Q': 
            cast_perm_ |= castling_permissions::c_wq;
            break;
        case 'k': 
            cast_perm_ |= castling_permissions::c_bk;
            break;
        case 'q': 
            cast_perm_ |= castling_permissions::c_bq;
            break;
        default:
            return false;
        }
        f++;
    }
    f++;

    ensure(cast_perm_ >= 0 && cast_perm_ < 16);
    ensure(*f != ' ');

    if(*f != '-') {
        const uint32 file = f[0] - 'a';
        const uint32 rank = f[1] - '1';

        ensure(file >= file::file_a && file <= file::file_h);
        ensure(rank >= rank::rank_1 && rank <= rank::rank_8);

        en_passant_sq_ = static_cast<square>(transition::fr_sq120(file, rank));
    }

    pos_key_ = generate_pos_key();
    return true;
}

uint64 engine::board::generate_pos_key() {
    uint64 key = 0;
    for(uint32 sq = 0; sq < N_BOARD_SQUARES_X; ++sq) {
        const auto p = b_[sq];
        if(p != square::no_sq &&p != square::offboard && p != piece_type::empty) {
            ensure(p >= piece_type::wp && p <= piece_type::bk);
            key ^= poskey::piece_keys[p][sq];
        }
    }

    if(side_ == side::white) {
        key ^= poskey::side_key;
    }

    if(en_passant_sq_ != square::no_sq) {
        ensure(en_passant_sq_ >= 0 && en_passant_sq_ < N_BOARD_SQUARES_X);
        key ^= poskey::piece_keys[piece_type::empty][en_passant_sq_];
    }

    ensure(cast_perm_ >= 0 && cast_perm_ < 16);
    key ^= poskey::castle_keys[cast_perm_];
    return key;
}

engine::square engine::board::king_of(const side side) {
    return king_sq_[side];
}

std::string engine::board::str() const {
    std::ostringstream stream;

    for(int32 rank = rank::rank_8; rank >= rank::rank_1; rank--) {
        stream << representation::ranks[rank] << " | ";
        for(int32 file = file::file_a; file <= file::file_h; file++) {
            const auto piece = b_[transition::fr_sq120(file, rank)];
            stream << std::setw(3) << std::left << representation::pieces[piece];
        }
        stream << '\n';
    }
    
    stream << "\n  ";
    for(int32 file = file::file_a; file <= file::file_h; file++)
        stream << "---";
    stream << "\n    ";
    for(int32 file = file::file_a; file <= file::file_h; file++)
        stream << std::setw(3) << std::left << representation::files[file];

    stream << "\nside: " << representation::sides[side_] << "\nen pas sq: " << en_passant_sq_;
    stream << "\ncastling: " 
        << (cast_perm_ & castling_permissions::c_wk ? 'K' : '-')
        << (cast_perm_ & castling_permissions::c_wq ? 'Q' : '-')
        << (cast_perm_ & castling_permissions::c_bk ? 'k' : '-')
        << (cast_perm_ & castling_permissions::c_bq ? 'q' : '-');
    stream << "\npos key: " << std::hex << pos_key_;

    return stream.str();
}
