// Copyright 2018 Emre Simsirli

#pragma once

#include "ObjectMacros.h"

UENUM(BlueprintType)
namespace EState
{
    enum Type
    {
        START UMETA(DisplayName = "Start"),
        MAIN_MENU UMETA(DisplayName = "Main Menu"),
        SEARCH_SERVER UMETA(DisplayName = "Searching Server"),
        CONNECT_SERVER UMETA(DisplayName = "Connecting Server"),
        LOADING_SCREEN UMETA(DisplayName = "Loading"),
        PLAYING UMETA(DisplayName = "Playing"),
        ERROR UMETA(DisplayName = "Error")
    };
}
