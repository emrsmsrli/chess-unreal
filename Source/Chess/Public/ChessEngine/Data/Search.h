// Copyright 2018 Emre Simsirli

#pragma once

#include "CoreTypes.h"
#include "ObjectMacros.h"
#include "Consts.h"
#include "Move.h"
#include "Search.generated.h"

USTRUCT()
struct CHESS_API FSearchInfo
{
    GENERATED_BODY()

    int32 starttime;
    int32 stoptime;
    int32 depth;
    int32 timeset;
    int32 movestogo;

    int64 nodes;

    int32 quit;
    int32 stopped;

    float fh; // fail high
    float fhf; // fail high first
    int32 nullCut;

    int32 GAME_MODE;
    int32 POST_THINKING;

    uint32 history[n_pieces][n_board_squares];
    FMove killers[2][max_depth];

    FSearchInfo();
    void AddKiller(uint32 ply, FMove& move);

    void Clear();
};
