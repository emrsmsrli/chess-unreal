// Copyright 2018 Emre Simsirli

#pragma once

#include "Side.h"
#include "Piece.h"
#include "Square.h"

class Verification
{
public:
    FORCEINLINE static bool IsSquareOnBoard(const uint32 sq)
    {
        return ESquare::Rank(sq) != ESquare::offboard;
    }

    FORCEINLINE static bool IsSideValid(const uint8 side)
    {
        return side == ESide::white || side == ESide::black;
    }

    FORCEINLINE static bool IsFileOrRankValid(const uint8 file_or_rank)
    {
        return file_or_rank >= 0 && file_or_rank <= 7;
    }

    FORCEINLINE static bool IsPieceValidOrEmpty(const uint32 piece)
    {
        return piece == EPieceType::empty || IsPieceValid(piece);
    }

    FORCEINLINE static bool IsPieceValid(const uint32 piece)
    {
        return piece >= EPieceType::wp && piece <= EPieceType::bk;
    }
};
