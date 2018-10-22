// Copyright 2018 Emre Simsirli

#include "Search.h"

FSearchInfo::FSearchInfo()
{
    Clear();
}

void FSearchInfo::AddKiller(const uint32 ply, FMove& move)
{
    killers[1][ply] = killers[0][ply];
    killers[0][ply] = move;
}

void FSearchInfo::Clear()
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
