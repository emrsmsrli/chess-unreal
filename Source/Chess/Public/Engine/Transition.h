#pragma once

#include "CoreMinimal.h"

namespace engine {
    namespace transition {
        void init();
        uint32 file_rank_sq120(uint32 file, uint32 rank);
        uint32 sq120(uint32 sq64);
        uint32 sq64(uint32 sq120);
    }
}
