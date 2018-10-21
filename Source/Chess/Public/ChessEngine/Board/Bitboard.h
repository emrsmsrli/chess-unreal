// Copyright 2018 Emre Simsirli

#pragma once

#include "CoreTypes.h"
#include "Debug.h"

class FString;

class CHESS_API FBitboard
{
    uint64 board_;
public:
    FBitboard();
    void SetSquare(uint32 sq);
    void ClearSquare(uint32 sq);
    uint32 Pop();
    int32 Count() const;
    void Empty();
    bool IsEmpty() const;

#ifdef DEBUG
    FString ToString() const;
#endif

    static void Initialize();
};
