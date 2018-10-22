// Copyright 2018 Emre Simsirli

#pragma once

#include "CoreTypes.h"
#include "ObjectMacros.h"
#include "Containers/Array.h"

enum EPieceType
{
    empty, wp, wn, wb, wr, wq, wk, bp, bn, bb, br, bq, bk
};

struct CHESS_API FPieceInfo
{
    bool bIsBig;
    bool bIsMajor;
    bool bIsMinor;
    int32 Value;
    int32 Side;

    bool bIsPawn;
    bool bIsKnight;
    bool bIsKing;
    bool bIsRookOrQueen;
    bool bIsBishopOrQueen;
    bool bIsSliding;

    TArray<int32> MoveDirections;

    FPieceInfo(const int32 v, const int32 s, const bool ikn,
           const bool ikg, const bool irq, const bool ibq, TArray<int32>&& mdirs)
        : Value(v), Side(s), bIsKnight(ikn), bIsKing(ikg),
          bIsRookOrQueen(irq), bIsBishopOrQueen(ibq), MoveDirections(MoveTemp(mdirs))
    {
        bIsPawn = Value && !bIsRookOrQueen && !bIsKnight && !bIsBishopOrQueen && !bIsKing;
        bIsBig = !bIsPawn;
        bIsMajor = bIsRookOrQueen || bIsKing;
        bIsMinor = bIsKnight || bIsBishopOrQueen && !bIsRookOrQueen;
        bIsSliding = !bIsPawn && !bIsKnight && !bIsKing;
    }
};

extern const FPieceInfo piece_infos[];
