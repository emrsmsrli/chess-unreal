// Copyright 2018 Emre Simsirli

#pragma once

#include "Object.h"
#include "Debug.h"
#include "ChessEngine.generated.h"

class UPrincipleVariationTable;
class UMoveGenerator;
class UEvaluator;
class FMove;
class UBoard;
struct FSearchInfo;

DECLARE_DELEGATE_OneParam(DMoveFoundDelegate, FMove)

UCLASS()
class CHESS_API UChessEngine : public UObject
{
    GENERATED_BODY()

    friend UPrincipleVariationTable;
    friend UMoveGenerator;
    friend UEvaluator;

    UBoard* board_;
    UMoveGenerator* move_generator_;
    UEvaluator* evaluator_;
    UPrincipleVariationTable* pv_table_;

public:
    bool bIsMultiplayer = true;
    FSearchInfo* SearchInfo;
    DMoveFoundDelegate MoveFoundDelegate;

    UChessEngine();
    void Set(FString& fen) const;

    void Search() const;
    // void SaveGame();
    // void LoadGame();

    static void Initialize();

#ifdef DEBUG
    FString Perft(int32 depth) const;

private:
    void Perft(int32 depth, int64* leaf_nodes) const;
#endif
};

extern UChessEngine* CEngine;
