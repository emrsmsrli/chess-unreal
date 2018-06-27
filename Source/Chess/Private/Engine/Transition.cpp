#include "Transition.h"
#include "Defs.h"

namespace engine {
    namespace transition {
        uint32 sq120_sq64[N_BOARD_SQUARES_X];
        uint32 sq64_sq120[N_BOARD_SQUARES];

        uint32 files[N_BOARD_SQUARES_X];
        uint32 ranks[N_BOARD_SQUARES_X];
    }
}

void engine::transition::init() {
    for(uint32 i = 0; i < N_BOARD_SQUARES_X; ++i) {
        sq120_sq64[i] = 65;

        files[i] = square::offboard;
        ranks[i] = square::offboard;
    }

    uint32 sq64 = 0;
    for(uint32 r = rank::rank_1; r < rank::rank_none; ++r) {
        for(uint32 f = file::file_a; f < file::file_none; ++f) {
            // init sq120_sq64, sq64_sq120
            const auto sq = fr_sq120(f, r);
            sq64_sq120[sq64] = sq;
            sq120_sq64[sq] = sq64;
            sq64++;
            
            // init files, ranks
            files[sq] = f;
            ranks[sq] = r;
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

engine::rank engine::transition::sq_rank(const uint32 sq) {
    return static_cast<rank>(ranks[sq]);
}

engine::file engine::transition::sq_file(const uint32 sq) {
    return static_cast<file>(files[sq]);
}
