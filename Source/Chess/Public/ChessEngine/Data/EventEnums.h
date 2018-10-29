// Copyright 2018 Emre Simsirli

#pragma once

#include "ObjectMacros.h"

UENUM(BlueprintType)
namespace EGameState
{
    enum Type
    {
        not_over,
        mate,
        draw
    };
}

UENUM(BlueprintType)
namespace EGameOverReason
{
    enum Type 
    {
        none,
        fifty_move,
        trifold_repetition,
        insufficent_material, // @see UBoard::IsDrawByMaterial
        mate_black,
        mate_white,
        stalemate
    };
}