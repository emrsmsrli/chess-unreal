// Copyright 2018 Emre Simsirli

#pragma once

#include "CoreMinimal.h"

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
    FString ToString() const;

    static void Initialize();
};
