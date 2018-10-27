#pragma once

#include "Debug.h"

#ifdef DEBUG
#include "Chess.h"

#define LOGI(f, ...) UE_LOG(LogChess, Log, \
    TEXT("[%s] - %s"), *FString(__FUNCTION__), *FString::Printf(TEXT(f), ##__VA_ARGS__))

#define LOGW(f, ...) UE_LOG(LogChess, Warning, \
    TEXT("[%s] - %s"), *FString(__FUNCTION__), *FString::Printf(TEXT(f), ##__VA_ARGS__)
#else 
#define LOGI(f, ...)
#define LOGW(f, ...)
#endif