// Copyright 2018 Emre Simsirli

#pragma once

#include "Debug.h"
#include "Object.h"
#include "Platform.h"
#include "ChessEngine.generated.h"

#ifdef DEBUG
#include "CString.h"
#endif

UCLASS()
class UChessEngine : public UObject
{
    GENERATED_BODY()

    friend class UPrincipleVariationTable;
    friend class UMoveGenerator;
    friend class UEvaluator;

    class UBoard* board_;
    class UMoveGenerator* move_generator_;
    class UEvaluator* evaluator_;
    class UPrincipleVariationTable* pv_table_;

public:
    bool bIsMultiplayer = true;
    struct FSearchInfo* search_info;

    UChessEngine();

    void Search() const;
    // void SaveGame();
    // void LoadGame();

    static void Initialize();

#ifdef DEBUG
    void Set(FString& fen) const;
    FString Perft(int32 depth) const;

private:
    void Perft(int32 depth, int64* leaf_nodes) const;
#endif
};

extern UChessEngine* CEngine;
