#include "Transition.h"
#include "ChessEngine.h"

namespace engine {
    namespace transition {
        uint32 sq120_sq64[N_BOARD_SQUARES_X];
        uint32 sq64_sq120[N_BOARD_SQUARES];
    }
}

void engine::transition::init() {
    for(uint32 i = 0; i < N_BOARD_SQUARES_X; ++i) {
        sq120_sq64[i] = 65;
    }

    uint32 sq64 = 0;
    for(uint32 r = engine::rank::rank_1; r < engine::rank::rank_none; ++r) {
        for(uint32 f = engine::file::file_a; f < engine::file::file_none; ++f) {
            const auto sq = file_rank_sq120(f, r);
            sq64_sq120[sq64] = sq;
            sq120_sq64[sq] = sq64;
            sq64++;
        }
    }
}

uint32 engine::transition::file_rank_sq120(const uint32 file, const uint32 rank) {
    return 21 + file + rank * 10;
}

uint32 engine::transition::sq120(const uint32 sq64) {
    return sq64_sq120[sq64];
}

uint32 engine::transition::sq64(const uint32 sq120) {
    return sq120_sq64[sq120];
}
