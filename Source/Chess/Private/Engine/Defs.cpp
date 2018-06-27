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
    {0, engine::side::both, false, false, false, false},

    {100, engine::side::white, false, false, false, false},
    {325, engine::side::white, true, false, false, false},
    {325, engine::side::white, false, false, false, true},
    {550, engine::side::white, false, false, true, false},
    {1000, engine::side::white, false, false, true, true},
    {50000, engine::side::white, false, true, false, false},

    {100, engine::side::black, false, false, false, false},
    {325, engine::side::black, true, false, false, false},
    {325, engine::side::black, false, false, false, true},
    {550, engine::side::black, false, false, true, false},
    {1000, engine::side::black, false, false, true, true},
    {50000, engine::side::black, false, true, false, false}
};
