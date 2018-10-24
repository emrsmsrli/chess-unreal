#pragma once

#include "LogMacros.h"

#define LOGI(Msg) UE_LOG(LogChess, Log, \
    TEXT("[%s] - %s"), __FUNCTION__, Msg)

#define LOGW(Msg) UE_LOG(LogChess, Warning, \
    TEXT("[%s] - %s"), __FUNCTION__, Msg)
