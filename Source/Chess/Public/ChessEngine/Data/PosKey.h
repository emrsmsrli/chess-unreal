// Copyright 2018 Emre Simsirli

#pragma once

#include "CoreTypes.h"

namespace PosKey
{
    uint64 GetPieceKey(uint32 piece_number, uint32 square);
    uint64 GetSideKey();
    uint64 GetCastleKey(uint32 permission);

    void Initialize();
}
