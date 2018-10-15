// Copyright 2018 Emre Simsirli

#pragma once

#include "ObjectMacros.h"

UENUM(BlueprintType)
namespace ESide
{
    enum Type
    {
        white = 0 UMETA(DisplayName = "White"),
        black UMETA(DisplayName = "Black"),
        both UMETA(DisplayName = "Both")
    };
}
