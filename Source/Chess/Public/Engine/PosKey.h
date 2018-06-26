#pragma once

#include "ChessEngine.h"
#include "CoreMinimal.h"

namespace engine {
    namespace poskey {
        extern uint64 piece_keys[N_PIECES][N_BOARD_SQUARES_X];
        extern uint64 side_key;
        extern uint64 castle_keys[16];

        void init();
    }
}