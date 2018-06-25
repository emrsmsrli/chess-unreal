#include "Board.h"
#include "PosKey.h"

uint64 engine::board::generate_pos_key() {
    uint64 key = 0;
    for(uint32 sq = 0; sq < N_BOARD_SQUARES_X; ++sq) {
        const auto p = b_[sq];
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
