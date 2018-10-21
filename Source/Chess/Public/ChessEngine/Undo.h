// Copyright 2018 Emre Simsirli

#pragma once

#include "Move.h"

struct FUndo
{
    FMove move;
    uint32 cast_perm;
    uint32 en_passant_sq;
    uint32 fifty_move_counter;
    uint64 pos_key;

    explicit FUndo(const FMove& m) : move(m)
    {
        cast_perm = 0;
        en_passant_sq = 0;
        fifty_move_counter = 0;
        pos_key = 0;
    }
};
