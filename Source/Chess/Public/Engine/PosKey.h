#pragma once

#include "CoreMinimal.h"

namespace engine {
    namespace poskey {
        extern uint64 piece_keys[13][120];
        extern uint64 side_key;
        extern uint64 castle_keys[16];

        void init();
    }
}