#include "Transition.h"
#include "Defs.h"

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
    for(uint32 r = rank::rank_1; r < rank::rank_none; ++r) {
        for(uint32 f = file::file_a; f < file::file_none; ++f) {
            const auto sq = fr_sq120(f, r);
            sq64_sq120[sq64] = sq;
            sq120_sq64[sq] = sq64;
            sq64++;
        }
    }
}

uint32 engine::transition::fr_sq120(const uint32 file, const uint32 rank) {
    return 21 + file + rank * 10;
}

uint32 engine::transition::fr_sq64(const uint32 file, const uint32 rank) {
    return sq64(fr_sq120(file, rank));
}

uint32 engine::transition::sq120(const uint32 sq64) {
    return sq64_sq120[sq64];
}

uint32 engine::transition::sq64(const uint32 sq120) {
    return sq120_sq64[sq120];
}
