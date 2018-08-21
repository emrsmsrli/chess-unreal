// Copyright 2018 Emre Simsirli

#pragma once

#include "ObjectMacros.h"

UENUM(BlueprintType)
namespace ETeam {
    enum Type {
        WHITE = 0   UMETA(DisplayName = "White"),
        BLACK       UMETA(DisplayName = "Black"),
        BOTH        UMETA(DisplayName = "Both")
    };
}
