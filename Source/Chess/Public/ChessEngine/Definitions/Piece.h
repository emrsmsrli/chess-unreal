// Copyright 2018 Emre Simsirli

#pragma once

#include "CoreTypes.h"
#include "Containers/Array.h"

enum EPieceType
{
    empty, wp, wn, wb, wr, wq, wk, bp, bn, bb, br, bq, bk
};

struct CHESS_API TPieceInfo
{
    bool is_big;
    bool is_major;
    bool is_minor;
    uint32 value;
    uint8 side;

    bool is_pawn;
    bool is_knight;
    bool is_king;
    bool is_rook_queen;
    bool is_bishop_queen;
    bool is_sliding;

    TArray<int32> move_directions;

    TPieceInfo(const uint32 v, const uint8 s, const bool ikn,
           const bool ikg, const bool irq, const bool ibq, const TArray<int32> mdirs)
        : value(v), side(s), is_knight(ikn), is_king(ikg),
          is_rook_queen(irq), is_bishop_queen(ibq), move_directions(mdirs)
    {
        is_pawn = value && !is_rook_queen && !is_knight && !is_bishop_queen && !is_king;
        is_big = !is_pawn;
        is_major = is_rook_queen || is_king;
        is_minor = is_knight || is_bishop_queen && !is_rook_queen;
        is_sliding = !is_pawn && !is_knight && !is_king;
    }
};

extern TPieceInfo piece_infos[];
