#include "PosKey.h"
#include <random>
#include <functional>
#include <chrono>
#include "CoreMinimal.h"

namespace engine {
    namespace poskey {
        uint64 piece_keys[N_PIECES][N_BOARD_SQUARES_X];
        uint64 side_key;
        uint64 castle_keys[16];
    }
}

auto create_uniform_random() {
    std::mt19937_64 generator{
        static_cast<uint64>(std::chrono::high_resolution_clock::now().time_since_epoch().count())
    };
    std::uniform_int_distribution<uint64> distribution;
    return std::bind(distribution, generator);
}

void engine::poskey::init() {
    auto rand = create_uniform_random();

    for(uint32 i = 0; i < N_PIECES; ++i)
        for(uint32 j = 0; j < N_BOARD_SQUARES_X; ++j)
            piece_keys[i][j] = rand();

    side_key = rand();

    for(uint32 i = 0; i < 16; ++i)
        castle_keys[i] = rand();
}
