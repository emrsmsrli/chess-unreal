#include "Defs.h"

namespace engine {
    namespace representation {
        char pieces[] = ".PNBRQKpnbrqk";
        char files[] = "abcdefgh";
        char ranks[] = "12345678";
        char sides[] = "wb-";
    }
}

engine::piece engine::pieces[] = {
    {false, false, false, 0, engine::side::both},

    {false, false, false, 100, engine::side::white},
    {true, false, true, 325, engine::side::white},
    {true, false, true, 325, engine::side::white},
    {true, true, false, 550, engine::side::white},
    {true, true, false, 1000, engine::side::white},
    {true, true, false, 50000, engine::side::white},

    {false, false, false, 100, engine::side::black},
    {true, false, true, 325, engine::side::black},
    {true, false, true, 325, engine::side::black},
    {true, true, false, 550, engine::side::black},
    {true, true, false, 1000, engine::side::black},
    {true, true, false, 50000, engine::side::black}
};
