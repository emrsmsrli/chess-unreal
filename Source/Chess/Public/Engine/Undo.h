#pragma once

#include "Move.h"

namespace engine {
    struct CHESS_API undo {
        move move;
        uint32 cast_perm;
        square en_passant_sq;
        uint32 fifty_move_counter;
        uint64 pos_key;

        undo(const engine::move& move, const uint32 cast_perm, const square en_passant_sq,
             const uint32 fifty_move_counter, const uint64 pos_key)
            : move(move),
              cast_perm(cast_perm),
              en_passant_sq(en_passant_sq),
              fifty_move_counter(fifty_move_counter),
              pos_key(pos_key) {}
    };
}
