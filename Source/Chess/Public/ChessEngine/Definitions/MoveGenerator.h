// Copyright 2018 Emre Simsirli

#pragma once

#include "Object.h"
#include "Containers/Array.h"
#include "Move.h"
#include "MoveGenerator.generated.h"

UCLASS()
class CHESS_API UMoveGenerator : public UObject
{
    GENERATED_BODY()

public:
    TArray<FMove> GenerateMoves() const;
    TArray<FMove> GenerateMoves(uint32 sq) const;
    bool DoesMoveExist(const FMove& m) const;

    static void Initialize();

private:
    void GeneratePawnMoves(uint32 sq, TArray<FMove>& moves) const;
    void GenerateSlidingMoves(uint32 sq, TArray<FMove>& moves) const;
    void GenerateNonSlidingMoves(uint32 sq, TArray<FMove>& moves) const;
    void GenerateCastlingMoves(TArray<FMove>& moves) const;

    void AddQuietMove(FMove move, TArray<FMove>& moves) const;
    void AddCaptureMove(FMove move, TArray<FMove>& moves) const;
    void AddEnPassantMove(FMove move, TArray<FMove>& moves) const;

    void AddPawnRegularMove(uint32 from, uint32 to, TArray<FMove>& moves) const;
    void AddPawnCaptureMove(uint32 from, uint32 to,
                               uint32 captured, TArray<FMove>& moves) const;
};
