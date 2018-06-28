#include "Board.h"
#include "Transition.h"
#include "PosKey.h"
#include "Defs.h"
#include <sstream>
#include <iomanip>

#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

namespace engine {
    namespace {
        const int32 direction_knight[] = {-8, -19, -21, -12, 8, 19, 21, 12};
        const int32 direction_rook[] = {-1, -10, 1, 10};
        const int32 direction_bishop[] = {-9, -11, 9, 11};
        const int32 direction_king[] = {-1, -10, -9, -11, 1, 10, 9, 11};
    }
}

engine::board::board() {
    set(START_FEN);
}

void engine::board::reset() {
    for(uint32 sq = 0; sq < N_BOARD_SQUARES_X; ++sq) {
        b_[sq] = square::offboard;
    }

    for(uint32 sq = 0; sq < N_BOARD_SQUARES; ++sq) {
        b_[transition::sq120(sq)] = piece_type::empty;
    }

    for(uint32 i = 0; i < 2; ++i) {
        n_big_pieces_[i] = 0;
        n_major_pieces_[i] = 0;
        n_minor_pieces_[i] = 0;
        material_score_[i] = 0;
    }

    for(uint32 i = 0; i < N_PIECES; ++i) {
        piece_count_[i] = 0;
    }

    for(uint32 side = side::white; side <= side::black; ++side)
        king_sq_[side] = square::no_sq;

    history_.clear();
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

        file += count;
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

        ensure(file >= file::file_a && file <= file::file_h);
        ensure(rank >= rank::rank_1 && rank <= rank::rank_8);

        en_passant_sq_ = static_cast<square>(transition::fr_sq120(file, rank));
    }

    pos_key_ = generate_pos_key();
    update_material();
    return true;
}

uint64 engine::board::generate_pos_key() {
    uint64 key = 0;
    for(uint32 sq = 0; sq < N_BOARD_SQUARES_X; ++sq) {
        const auto p = b_[sq];
        if(p != square::no_sq && p != square::offboard && p != piece_type::empty) {
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

void engine::board::update_material() {
    for(uint32 sq = 0; sq < N_BOARD_SQUARES_X; ++sq) {
        const auto piece = b_[sq];
        if(piece != square::offboard && piece != piece_type::empty) {
            const auto p = pieces[piece];
            const auto side = p.side;

            if(p.is_big) n_big_pieces_[side]++;
            if(p.is_major) n_major_pieces_[side]++;
            if(p.is_minor) n_minor_pieces_[side]++;

            material_score_[side] += p.value;

            piece_list_[piece][piece_count_[piece]] = static_cast<square>(sq);
            piece_count_[piece]++;

            switch(piece) {
            case piece_type::wk:
                king_sq_[side::white] = static_cast<square>(sq);
                break;
            case piece_type::bk:
                king_sq_[side::black] = static_cast<square>(sq);
                break;
            case piece_type::wp:
                pawns_[side::white].set_sq(transition::sq64(sq));
                pawns_[side::both].set_sq(transition::sq64(sq));
                break;
            case piece_type::bp:
                pawns_[side::black].set_sq(transition::sq64(sq));
                pawns_[side::both].set_sq(transition::sq64(sq));
                break;
            default: break;
            }
        }
    }
}

engine::square engine::board::king_of(const side side) {
    return king_sq_[side];
}

bool engine::board::is_attacked(const square sq, const side side) {
    // pawns
    if(side == side::white) {
        if(b_[sq - 11] == piece_type::wp || b_[sq - 9] == piece_type::wp)
            return true;
    } else {
        if(b_[sq + 11] == piece_type::bp || b_[sq + 9] == piece_type::bp)
            return true;
	}

    // knights
    for(uint32 i = 0; i < 8; ++i) {
        const auto piece = pieces[b_[static_cast<int>(sq) + direction_knight[i]]];
        if(piece.is_knight && piece.side == side)
            return true;
    }

    // rooks & queens
    for(uint32 i = 0; i < 4; ++i) {
        const auto d = direction_rook[i];
        auto sqq = sq + d;
        auto piece = b_[sqq];
        while(piece != square::offboard) {
            if(piece != piece_type::empty) {
                if(pieces[piece].is_rook_queen && pieces[piece].side == side)
                    return true;
                break;
            }
            sqq += d;
            piece = b_[sqq];
        }
    }

    // bishops & queens
    for(uint32 i = 0; i < 4; ++i) {
        const auto d = direction_bishop[i];
        auto sqq = sq + d;
        auto piece = b_[sqq];
        while(piece != square::offboard) {
            if(piece != piece_type::empty) {
                if(pieces[piece].is_bishop_queen && pieces[piece].side == side)
                    return true;
                break;
            }
            sqq += d;
            piece = b_[sqq];
        }
    }

    // kings
    for(uint32 i = 0; i < 4; ++i) {
        const auto piece = b_[sq + direction_king[i]];
        if(pieces[piece].is_king && pieces[piece].side == side)
            return true;
    }

    return false;
}

bool engine::board::is_valid() {
    uint32 piece_count[N_PIECES];
	uint32 n_big_pieces[2] = {0, 0};
    uint32 n_major_pieces[2] = {0, 0};
    uint32 n_minor_pieces[2] = {0, 0};
    uint32 material_score[2] = {0, 0};

    bitboard pawns[3];
    pawns[0] = static_cast<uint64>(pawns_[0]);
    pawns[1] = static_cast<uint64>(pawns_[1]);
    pawns[2] = static_cast<uint64>(pawns_[2]);

    for(uint32 sq = 0; sq < N_PIECES; sq++)
        piece_count[sq] = 0;

    for(uint32 p = piece_type::wp; p <= piece_type::bk; ++p) {
        for(uint32 n_p = 0; n_p < piece_count_[p]; ++n_p) {
            const auto sq120 = piece_list_[p][n_p];
            ensure(b_[sq120] == p);
        }
    }

    for(uint32 sq64 = 0; sq64 < N_BOARD_SQUARES; sq64++) {
        const auto sq120 = transition::sq120(sq64);
        const auto p = b_[sq120];
        piece_count[p]++;

        const auto side = pieces[p].side;
        if(pieces[p].is_big) n_big_pieces[side]++;
        if(pieces[p].is_major) n_major_pieces[side]++;
        if(pieces[p].is_minor) n_minor_pieces[side]++;

        material_score[side] += pieces[p].value;
    }

    for(uint32 p = piece_type::wp; p <= piece_type::bk; ++p) {
        ensure(piece_count[p] == piece_count_[p]);
    }

    ensure(pawns[side::white].count() == piece_count_[piece_type::wp]);
    ensure(pawns[side::black].count() == piece_count_[piece_type::bp]);
    ensure(pawns[side::both].count() == piece_count_[piece_type::wp] + piece_count_[piece_type::bp]);

    while(!pawns[side::white].is_empty()) {
        const auto sq64 = pawns[side::white].pop();
        ensure(b_[transition::sq120(sq64)] == piece_type::wp);
    }

    while(!pawns[side::black].is_empty()) {
        const auto sq64 = pawns[side::black].pop();
        ensure(b_[transition::sq120(sq64)] == piece_type::bp);
    }

    while(!pawns[side::both].is_empty()) {
        const auto sq64 = pawns[side::both].pop();
        ensure(b_[transition::sq120(sq64)] == piece_type::wp || b_[transition::sq120(sq64)] == piece_type::bp);
    }

    ensure(material_score[side::white] == material_score_[side::white] 
		   && material_score[side::black] == material_score_[side::black]);
    ensure(n_minor_pieces[side::white] == n_minor_pieces_[side::white] 
		   && n_minor_pieces[side::black] == n_minor_pieces_[side::black]);
    ensure(n_major_pieces[side::white] == n_major_pieces_[side::white] 
		   && n_major_pieces[side::black] == n_major_pieces_[side::black]);
    ensure(n_big_pieces[side::white] == n_big_pieces_[side::white] 
		   && n_big_pieces[side::black] == n_big_pieces_[side::black]);
    ensure(side_ == side::white || side_ == side::black);
    ensure(generate_pos_key() == pos_key_);

    ensure(en_passant_sq_ == square::no_sq ||
        transition::sq_rank(en_passant_sq_) == rank::rank_6 && side_ == side::white ||
        transition::sq_rank(en_passant_sq_) == rank::rank_3 && side_ == side::black);

    ensure(b_[king_sq_[side::white]] == piece_type::wk);
    ensure(b_[king_sq_[side::black]] == piece_type::bk);

    return true;
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

    // todo remove this -- stream << "\nwp:\n" << pawns_[side::white].str() << "\nbp:\n" << pawns_[side::black].str() << '\n';

    return stream.str();
}
