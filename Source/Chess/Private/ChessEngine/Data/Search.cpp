// Copyright 2018 Emre Simsirli

#include "Search.h"

FSearchInfo::FSearchInfo()
{
    Clear();
}

void FSearchInfo::AddKiller(const uint32 ply, FMove& move)
{
    Killers[1][ply] = Killers[0][ply];
    Killers[0][ply] = move;
}

void FSearchInfo::AddHistory(const uint32 piece, const uint32 sq, const uint32 depth)
{
    History[piece][sq] += depth;
}

FMove FSearchInfo::GetKiller(const uint32 index, const uint32 ply)
{
    return Killers[index][ply];
}

uint32 FSearchInfo::GetHistory(const uint32 piece, const uint32 sq)
{
    return History[piece][sq];
}

void FSearchInfo::Clear()
{
    bStopRequested = false;
    StartTime = 0;
    StopTimeSet = 0;
    StopTimeActual = 0;

    TotalVisitedNodes = 0;

#ifdef DEBUG
    F_H = 0;
    F_H_F = 0;
#endif

    for(auto& i : History) {
        for(auto& j : i) {
            j = 0;
        }
    }

    for(auto& i : Killers) {
        for(auto& j : i) {
            j = FMove::no_move;
        }
    }
}
