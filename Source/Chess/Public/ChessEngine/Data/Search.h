// Copyright 2018 Emre Simsirli

#pragma once

#include "CoreTypes.h"
#include "ObjectMacros.h"
#include "Consts.h"
#include "Move.h"
#include "Search.generated.h"

USTRUCT(BlueprintType)
struct CHESS_API FSearchParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Difficulty", 
		meta = (ClampMax = 6, ClampMin = 1, ToolTip = "Max depth the search can go"))
    int32 Depth = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Difficulty", 
		meta = (ClampMax = 30, ClampMin = 0, ToolTip = "Search will stop after this seconds"))
    float TimeSet = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chess|Difficulty", 
		meta = (ToolTip = "Should the search use null move cut"))
    bool UseNullCut = true;
};

struct CHESS_API FSearchInfo
{
    float StartTime = 0;
    float StopTimeSet = 0;
    float StopTimeActual = 0;

    bool bStopRequested = false;

    int64 TotalVisitedNodes = 0;
    
    // fail high
    float F_H = 0;
    // fail high first
    float F_H_F = 0; 

    // heuristics for better move ordering
    uint32 History[n_pieces][n_board_squares]{};
    FMove Killers[2][max_depth];

    FSearchInfo();
    void AddKiller(uint32 ply, FMove& move);
    void AddHistory(uint32 piece, uint32 sq, uint32 depth);
    FMove GetKiller(uint32 index, uint32 ply);
    uint32 GetHistory(uint32 piece, uint32 sq);

    void Clear();
};
