#include "ChessEngine.h"
#include "Transition.h"
#include "Bitboard.h"
#include "Defs.h"
#include "PosKey.h"
#include "Verify.h"
#include "Board.h"

namespace {
    engine::board* b = nullptr;
}

engine::move engine::parse_move(const std::string& mstr) {
    const auto mchars = mstr.c_str();
    if(mchars[0] > 'h' || mchars[0] < 'a'
        || mchars[1] > '8' || mchars[1] < '1'
        || mchars[2] > 'h' || mchars[2] < 'a'
        || mchars[3] > '8' || mchars[3] < '1') {
        return engine::move::no_move;
    }

    const auto from = transition::fr_sq120(mchars[0] - 'a', mchars[1] - '1');
    const auto to = transition::fr_sq120(mchars[2] - 'a', mchars[3] - '1');

    ensure(SQ_ON_BOARD(from) && SQ_ON_BOARD(to));

    const auto moves = b->generate_moves();
    for(auto& m : moves) {
        if(m.from() == from && m.to() == to) {
            const auto promoted = m.promoted_piece();
            if(promoted != piece_type::empty) {
                const auto p = pieces[promoted];
                if(p.is_rook_queen && !p.is_bishop_queen && mchars[4] == 'r'
                    || !p.is_rook_queen && p.is_bishop_queen && mchars[4] == 'b'
                    || p.is_rook_queen && p.is_bishop_queen && mchars[4] == 'q'
                    || p.is_knight && mchars[4] == 'n') {
                    return m;
                }
                continue;
            }
            return m;
        }
    }

    return move::no_move;
}

void engine::init() {
    transition::init();
    bitmask::init();
    poskey::init();
    b = new engine::board;
}
