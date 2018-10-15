// Copyright 2018 Emre Simsirli

#pragma once

#include "ObjectMacros.h"

UENUM()
namespace EFile
{
    enum Type
    {
        file_a, file_b, file_c, file_d, file_e, file_f, file_g, file_h, file_none
    };
}

UENUM()
namespace ERank
{
    enum Type
    {
        rank_1, rank_2, rank_3, rank_4, rank_5, rank_6, rank_7, rank_8, rank_none
    };
}

UENUM(BlueprintType)
namespace ESquare
{
    enum Type
    {
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

namespace EFile
{
    FString AsString(uint32 file);
}

namespace ERank
{
    FString AsString(uint32 rank);
}

namespace ESquare
{
    uint32 Rank(uint32 sq);
    uint32 File(uint32 sq);

    uint32 Sq120(uint32 file, uint32 rank);
    uint32 Sq64(uint32 file, uint32 rank);

    uint32 Sq120(uint32 sq64);
    uint32 Sq64(uint32 sq120);

    FString AsString(uint32 sq);

    void Initialize();
}
