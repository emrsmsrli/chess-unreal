#include "PosKey.h"
#include <random>
#include <functional>
#include "CoreMinimal.h"

namespace engine {
    namespace poskey {
        uint64 piece_keys[13][120];
        uint64 side_key;
        uint64 castle_keys[16];
    }
}

void engine::poskey::init() {
    std::random_device r;
    std::default_random_engine generator(r());
    std::uniform_int_distribution<uint64> distribution;
    auto rand = std::bind(distribution, generator);

    for(uint32 i = 0; i < 13; ++i)
        for(uint32 j = 0; j < 120; ++j)
            piece_keys[i][j] = rand();

    side_key = rand();

    for(uint32 i = 0; i < 16; ++i)
        castle_keys[i] = rand();
}
