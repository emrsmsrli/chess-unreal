#pragma once

#include "Move.h"

namespace engine {
    struct CHESS_API undo {
        move move;
        uint32 cast_perm;
        square en_passant_sq;
        uint32 fifty_move_counter;
        uint64 pos_key;

        undo(const engine::move& m) : move(m) {} 
    };
}
