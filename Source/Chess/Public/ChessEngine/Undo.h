// Copyright 2018 Emre Simsirli

#pragma once

#include "ChessEngine/Move.h"

struct FUndo {
    TMove move;
    uint32 cast_perm;
    uint32 en_passant_sq;
    uint32 fifty_move_counter;
    uint64 pos_key;

    FUndo(const TMove& m) : move(m) {}
};
