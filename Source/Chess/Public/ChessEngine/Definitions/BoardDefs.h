// Copyright 2018 Emre Simsirli

#pragma once

#include "Platform.h"
#include "Consts.h"
#include "Move.h"

UENUM()
enum ECastlingPermission
{
    c_wk = 1, c_wq = 2, c_bk = 4, c_bq = 8
};

struct FSearchInfo
{
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

FORCEINLINE FSearchInfo::FSearchInfo()
{
    Clear();
}

FORCEINLINE void FSearchInfo::AddKiller(const uint32 ply, FMove& move)
{
    killers[1][ply] = killers[0][ply];
    killers[0][ply] = move;
}

FORCEINLINE void FSearchInfo::Clear()
{
    starttime = 0;
    stoptime = 0;
    depth = 0;
    timeset = 0;
    movestogo = 0;

    nodes = 0;

    quit = 0;
    stopped = 0;

    fh = 0;
    fhf = 0;
    nullCut = 0;

    GAME_MODE = 0;
    POST_THINKING = 0;

    for(auto& i : history) {
        for(auto& j : i) {
            j = 0;
        }
    }

    for(auto& i : killers) {
        for(auto& j : i) {
            j = FMove::no_move;
        }
    }
}
