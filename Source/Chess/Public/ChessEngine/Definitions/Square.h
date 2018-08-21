// Copyright 2018 Emre Simsirli

#pragma once

#include "ObjectMacros.h"

UENUM(BlueprintType)
namespace ESquare {
    enum Type {
        a1 = 21, b1, c1, d1, e1, f1, g1, h1,
        a2 = 31, b2, c2, d2, e2, f2, g2, h2,
        a3 = 41, b3, c3, d3, e3, f3, g3, h3,
        a4 = 51, b4, c4, d4, e4, f4, g4, h4,
        a5 = 61, b5, c5, d5, e5, f5, g5, h5,
        a6 = 71, b6, c6, d6, e6, f6, g6, h6,
        a7 = 81, b7, c7, d7, e7, f7, g7, h7,
        a8 = 91, b8, c8, d8, e8, f8, g8, h8, 
        no_sq, offboard
    };
}

namespace Square {
    uint32 Rank(uint32 sq);
    uint32 File(uint32 sq);
    
    void Initialize();
}
