// Copyright 2018 Emre Simsirli

#pragma once

#include "CoreTypes.h"

class FString;

class CHESS_API FMove
{
public:
    static const uint32 flag_en_passant = 0x40000;
    static const uint32 flag_pawn_start = 0x80000;
    static const uint32 flag_castling = 0x1000000;

    static const FMove no_move;

private:
    uint32 move_;
    uint32 score_ = 0;

public:
    FMove() : FMove(0) {}
    explicit FMove(uint32 m);

    uint32 From() const;
    uint32 To() const;

    uint32 CapturedPiece() const;
    uint32 PromotedPiece() const;

    bool IsCaptured() const;
    bool IsPromoted() const;
    bool IsEnPassant() const;
    bool IsPawnStart() const;
    bool IsCastling() const;

    void SetScore(uint32 s);
    uint32 GetScore() const;

    static FMove Create(uint32 from, uint32 to, uint32 captured, uint32 promoted, uint32 flags);

    FString ToString() const;
    bool operator==(const FMove& o) const;
    bool operator!=(const FMove& o) const;

private:
    uint32 get(uint32 shift, uint32 mask) const;
};
