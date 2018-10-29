// Copyright 2018 Emre Simsirli

#pragma once

#include "Object.h"
#include "Debug.h"
#include "Search.h"
#include "ChessEngine.generated.h"

class UPrincipleVariationTable;
class UMoveGenerator;
class UMoveExplorer;
class FMoveExplorerThread;
class FMove;
class UBoard;
struct FSearchInfo;

DECLARE_DELEGATE_OneParam(FMoveFoundDelegate, FMove)

UCLASS()
class CHESS_API UChessEngine : public UObject
{
    GENERATED_BODY()

    friend UPrincipleVariationTable;
    friend UMoveGenerator;
    friend UMoveExplorer;
    friend FMoveExplorerThread;

    UBoard* board_;
    UMoveGenerator* move_generator_;
    UMoveExplorer* move_explorer_;
    FMoveExplorerThread* move_explorer_thread_;
    UPrincipleVariationTable* pv_table_;

public:
    bool bIsMultiplayer = true;
    FSearchInfo* SearchInfo;
    FSearchParams SearchParams;
    FMoveFoundDelegate MoveFoundDelegate;

    UChessEngine();
    void Set(FString& fen) const;

    void MakeMove(FMove& move) const;
    void TakeMove() const;
    TArray<FMove> GenerateMoves(uint32 sq) const;
    void Search() const;
    // void SaveGame();
    // void LoadGame();

    static void Initialize();
    static void Shutdown();

private:
    void CheckGameOver();

#ifdef DEBUG
public:
    FString Perft(int32 depth) const;

private:
    void Perft(int32 depth, int64* leaf_nodes) const;
#endif
};

extern UChessEngine* CEngine;
