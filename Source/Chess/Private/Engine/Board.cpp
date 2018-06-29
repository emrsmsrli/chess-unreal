#include "Board.h"
#include "Transition.h"
#include "PosKey.h"
#include "Defs.h"
#include "Verify.h"
#include "PosKey.h"
#include <sstream>
#include <iomanip>

#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define MAX_POSITION_MOVES 256

#define HASH_PIECE(p, sq)   (pos_key_ ^= poskey::piece_keys[p][sq])
#define HASH_CASTL()        (pos_key_ ^= poskey::castle_keys[cast_perm_])
#define HASH_SIDE()         (pos_key_ ^= poskey::side_key)
#define HASH_EN_P()         (pos_key_ ^= poskey::piece_keys[piece_type::empty][en_passant_sq_])

namespace engine {
    namespace {
        const int32 direction_knight[] = {-8, -19, -21, -12, 8, 19, 21, 12};
        const int32 direction_rook[] = {-1, -10, 1, 10};
        const int32 direction_bishop[] = {-9, -11, 9, 11};
        const int32 direction_king[] = {-1, -10, -9, -11, 1, 10, 9, 11};

        const int32 l_sliding_pieces[] = {
            piece_type::wb, piece_type::wr, piece_type::wq, 0,
            piece_type::bb, piece_type::br, piece_type::bq, 0
        };

        const int32 l_non_sliding_pieces[] = {
            piece_type::wn, piece_type::wk, 0,
            piece_type::bn, piece_type::bk, 0
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


        // video 38
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

    for(uint32 i = 0; i < 3; i++)
        pawns_[i] = 0;

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

bool engine::board::is_attacked(const square sq, const side side) {
    ensure(SQ_ON_BOARD(sq));
    ensure(SIDE_VALID(side));
    ensure(is_valid());

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

void engine::board::add_piece(const square sq, const piece_type piece) {
    ensure(SQ_ON_BOARD(sq));
    ensure(PIECE_VALID(piece));

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
        pawns_[p.side].set_sq(transition::sq64(sq));
        pawns_[side::both].set_sq(transition::sq64(sq));
    }
    material_score_[p.side] += p.value;
    piece_list_[piece][piece_count_[piece]++] = sq;
}

void engine::board::move_piece(const square from, const square to) {
    ensure(SQ_ON_BOARD(from));
    ensure(SQ_ON_BOARD(to));

    const auto p = b_[from];
    const auto pp = pieces[p];

    HASH_PIECE(p, from);
    b_[from] = piece_type::empty;
    HASH_PIECE(p, to);
    b_[from] = p;

    if(!pp.is_big) {
        pawns_[pp.side].clr_sq(transition::sq64(from));
        pawns_[side::both].clr_sq(transition::sq64(from));
        pawns_[pp.side].clr_sq(transition::sq64(to));
        pawns_[side::both].clr_sq(transition::sq64(to));
    }

    for(uint32 i = 0; i < piece_count_[p]; ++i) {
        if(piece_list_[p][i] == from) {
            piece_list_[p][i] = to;
            break;
        }
    }
}

void engine::board::clear_piece(const square sq) {
    ensure(SQ_ON_BOARD(sq));
    const auto p = b_[sq];
    const auto pp = pieces[p];
    ensure(PIECE_VALID(p));

    HASH_PIECE(p, sq);
    b_[sq] = piece_type::empty;
    material_score_[pp.side] -= pp.value;

    if(pp.is_big) {
        n_big_pieces_[pp.side]--;
        if(pp.is_major) {
            n_major_pieces_[pp.side]--;
        } else {
            n_minor_pieces_[pp.side]--;
        }
    } else {
        pawns_[pp.side].clr_sq(transition::sq64(sq));
        pawns_[side::both].clr_sq(transition::sq64(sq));
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

void engine::board::add_white_pawn_capture_move(const square from, const square to,
                                                const piece_type captured, std::vector<engine::move>* moves) {
    ensure(PIECE_VALID_EMPTY(captured));
    ensure(SQ_ON_BOARD(from));
    ensure(SQ_ON_BOARD(to));

    if(transition::sq_rank(from) == rank::rank_7) {
        add_capture_move(move::create(from, to, captured, piece_type::wq, 0), moves);
        add_capture_move(move::create(from, to, captured, piece_type::wr, 0), moves);
        add_capture_move(move::create(from, to, captured, piece_type::wb, 0), moves);
        add_capture_move(move::create(from, to, captured, piece_type::wn, 0), moves);
    } else {
        add_capture_move(move::create(from, to, captured, piece_type::empty, 0), moves);
    }
}

void engine::board::add_white_pawn_move(const square from, const square to, std::vector<engine::move>* moves) {
    ensure(SQ_ON_BOARD(from));
    ensure(SQ_ON_BOARD(to));

    if(transition::sq_rank(from) == rank::rank_7) {
        add_quiet_move(move::create(from, to, piece_type::empty, piece_type::wq, 0), moves);
        add_quiet_move(move::create(from, to, piece_type::empty, piece_type::wr, 0), moves);
        add_quiet_move(move::create(from, to, piece_type::empty, piece_type::wb, 0), moves);
        add_quiet_move(move::create(from, to, piece_type::empty, piece_type::wn, 0), moves);
    } else {
        add_quiet_move(move::create(from, to, piece_type::empty, piece_type::empty, 0), moves);
    }
}

void engine::board::add_black_pawn_capture_move(const square from, const square to,
                                                const piece_type captured, std::vector<engine::move>* moves) {
    ensure(PIECE_VALID_EMPTY(captured));
    ensure(SQ_ON_BOARD(from));
    ensure(SQ_ON_BOARD(to));

    if(transition::sq_rank(from) == rank::rank_2) {
        add_capture_move(move::create(from, to, captured, piece_type::bq, 0), moves);
        add_capture_move(move::create(from, to, captured, piece_type::br, 0), moves);
        add_capture_move(move::create(from, to, captured, piece_type::bb, 0), moves);
        add_capture_move(move::create(from, to, captured, piece_type::bn, 0), moves);
    } else {
        add_capture_move(move::create(from, to, captured, piece_type::empty, 0), moves);
    }
}

void engine::board::add_black_pawn_move(const square from, const square to, std::vector<engine::move>* moves) {
    ensure(SQ_ON_BOARD(from));
    ensure(SQ_ON_BOARD(to));

    if(transition::sq_rank(from) == rank::rank_2) {
        add_quiet_move(move::create(from, to, piece_type::empty, piece_type::bq, 0), moves);
        add_quiet_move(move::create(from, to, piece_type::empty, piece_type::br, 0), moves);
        add_quiet_move(move::create(from, to, piece_type::empty, piece_type::bb, 0), moves);
        add_quiet_move(move::create(from, to, piece_type::empty, piece_type::bn, 0), moves);
    } else {
        add_quiet_move(move::create(from, to, piece_type::empty, piece_type::empty, 0), moves);
    }
}

std::vector<engine::move>* engine::board::generate_moves() {
    ensure(is_valid());
    const auto moves = new std::vector<engine::move>;

    if(side_ == side::white) {
        for(uint32 p_count = 0; p_count < piece_count_[piece_type::wp]; ++p_count) {
            const auto sq = piece_list_[piece_type::wp][p_count];
            ensure(SQ_ON_BOARD(sq));

            if(b_[sq + 10] == piece_type::empty) {
                add_white_pawn_move(sq, static_cast<square>(sq + 10), moves);
                if(transition::sq_rank(sq) == rank::rank_2 && b_[sq + 20] == piece_type::empty)
                    add_quiet_move(move::create(sq, static_cast<square>(sq + 20), piece_type::empty,
                                                piece_type::empty, move::flag_pawn_start), moves);
            }

            if(SQ_ON_BOARD(sq + 9) && pieces[b_[sq + 9]].side == side::black)
                add_white_pawn_capture_move(sq, static_cast<square>(sq + 9), static_cast<piece_type>(b_[sq + 9]),
                                            moves);
            if(SQ_ON_BOARD(sq + 11) && pieces[b_[sq + 11]].side == side::black)
                add_white_pawn_capture_move(sq, static_cast<square>(sq + 11), static_cast<piece_type>(b_[sq + 11]),
                                            moves);

            if(sq + 9 == en_passant_sq_)
                add_capture_move(move::create(sq, static_cast<square>(sq + 9), piece_type::empty, piece_type::empty,
                                              move::flag_en_passant), moves);
            if(sq + 11 == en_passant_sq_)
                add_capture_move(move::create(sq, static_cast<square>(sq + 11), piece_type::empty, piece_type::empty,
                                              move::flag_en_passant), moves);
        }

        if(cast_perm_ & castling_permissions::c_wk) {
            if(b_[square::f1] == piece_type::empty && b_[square::g1] == piece_type::empty) {
                if(!is_attacked(square::e1, side::black) && !is_attacked(square::f1, side::black)) {
                    add_quiet_move(move::create(square::e1, square::g1, piece_type::empty, piece_type::empty,
                                                move::flag_castling), moves);
                }
            }
        }

        if(cast_perm_ & castling_permissions::c_wq) {
            if(b_[square::d1] == piece_type::empty && b_[square::c1] == piece_type::empty && b_[square::b1] ==
                piece_type::empty) {
                if(!is_attacked(square::e1, side::black) && !is_attacked(square::d1, side::black)) {
                    add_quiet_move(move::create(square::e1, square::c1, piece_type::empty, piece_type::empty,
                                                move::flag_castling), moves);
                }
            }
        }
    } else {
        for(uint32 p_count = 0; p_count < piece_count_[piece_type::bp]; ++p_count) {
            const auto sq = piece_list_[piece_type::bp][p_count];
            ensure(SQ_ON_BOARD(sq));

            if(b_[sq - 10] == piece_type::empty) {
                add_black_pawn_move(sq, static_cast<square>(sq - 10), moves);
                if(transition::sq_rank(sq) == rank::rank_7 && b_[sq - 20] == piece_type::empty)
                    add_quiet_move(move::create(sq, static_cast<square>(sq - 20), piece_type::empty,
                                                piece_type::empty, move::flag_pawn_start), moves);
            }

            if(SQ_ON_BOARD(sq - 9) && pieces[b_[sq - 9]].side == side::white)
                add_black_pawn_capture_move(sq, static_cast<square>(sq - 9), static_cast<piece_type>(b_[sq - 9]),
                                            moves);
            if(SQ_ON_BOARD(sq - 11) && pieces[b_[sq - 11]].side == side::white)
                add_black_pawn_capture_move(sq, static_cast<square>(sq - 11), static_cast<piece_type>(b_[sq - 11]),
                                            moves);

            if(sq - 9 == en_passant_sq_)
                add_capture_move(move::create(sq, static_cast<square>(sq - 9), piece_type::empty, piece_type::empty,
                                              move::flag_en_passant), moves);
            if(sq - 11 == en_passant_sq_)
                add_capture_move(move::create(sq, static_cast<square>(sq - 11), piece_type::empty, piece_type::empty,
                                              move::flag_en_passant), moves);
        }

        if(cast_perm_ & castling_permissions::c_bk) {
            if(b_[square::f8] == piece_type::empty && b_[square::g8] == piece_type::empty) {
                if(!is_attacked(square::e8, side::white) && !is_attacked(square::f8, side::white)) {
                    add_quiet_move(move::create(square::e8, square::g8, piece_type::empty, piece_type::empty,
                                                move::flag_castling), moves);
                }
            }
        }

        if(cast_perm_ & castling_permissions::c_bq) {
            if(b_[square::d8] == piece_type::empty && b_[square::c8] == piece_type::empty && b_[square::b8] ==
                piece_type::empty) {
                if(!is_attacked(square::e8, side::white) && !is_attacked(square::d8, side::white)) {
                    add_quiet_move(move::create(square::e8, square::c8, piece_type::empty, piece_type::empty,
                                                move::flag_castling), moves);
                }
            }
        }
    }

    for(uint32 s_i = l_slide_index[side_]; ; s_i++) {
        const auto piece = l_sliding_pieces[s_i];
        if(piece == piece_type::empty)
            break;
        ensure(PIECE_VALID(piece));

        for(uint32 p_n = 0; p_n < piece_count_[piece]; ++p_n) {
            const auto sq = piece_list_[piece][p_n];
            ensure(SQ_ON_BOARD(sq));

            for(uint32 i = 0; i < num_dir[piece]; ++i) {
                const auto dir = piece_directions[piece][i];
                auto sqq = sq + dir;

                while(SQ_ON_BOARD(sqq)) {
                    if(b_[sqq] != piece_type::empty) {
                        if(pieces[b_[sqq]].side == (side_ ^ 1))
                            add_capture_move(move::create(sq, static_cast<square>(sqq),
                                                          static_cast<piece_type>(b_[sqq]), piece_type::empty, 0),
                                             moves);
                        break;
                    }
                    add_quiet_move(move::create(sq, static_cast<square>(sqq),
                                                piece_type::empty, piece_type::empty, 0), moves);
                    sqq += dir;
                }
            }
        }
    }

    for(uint32 s_i = l_non_slide_index[side_]; ; s_i++) {
        const auto piece = l_non_sliding_pieces[s_i];
        if(piece == piece_type::empty)
            break;
        ensure(PIECE_VALID(piece));

        for(uint32 p_n = 0; p_n < piece_count_[piece]; ++p_n) {
            const auto sq = piece_list_[piece][p_n];
            ensure(SQ_ON_BOARD(sq));

            for(uint32 i = 0; i < num_dir[piece]; ++i) {
                const auto dir = piece_directions[piece][i];
                const auto sqq = sq + dir;

                if(!SQ_ON_BOARD(sqq))
                    continue;

                if(b_[sqq] != piece_type::empty) {
                    if(pieces[b_[sqq]].side == (side_ ^ 1))
                        add_capture_move(move::create(sq, static_cast<square>(sqq),
                                                      static_cast<piece_type>(b_[sqq]), piece_type::empty, 0), moves);
                    continue;
                }
                add_quiet_move(move::create(sq, static_cast<square>(sqq),
                                            piece_type::empty, piece_type::empty, 0), moves);
            }
        }
    }

    return moves;
}

void engine::board::add_quiet_move(move* move, std::vector<engine::move>* moves) {
    moves->push_back(*move);
}

void engine::board::add_capture_move(move* move, std::vector<engine::move>* moves) {
    moves->push_back(*move);
}

void engine::board::add_en_passant_move(move* move, std::vector<engine::move>* moves) {
    moves->push_back(*move);
}
