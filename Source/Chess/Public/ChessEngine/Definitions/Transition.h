// Copyright 2018 Emre Simsirli

#pragma once

#include "CoreMinimal.h"

namespace Transition {
    uint32 fr_sq120(uint32 file, uint32 rank);
    uint32 fr_sq64(uint32 file, uint32 rank);
    uint32 sq120(uint32 sq64);
    uint32 sq64(uint32 sq120);

    void Initialize();
}
