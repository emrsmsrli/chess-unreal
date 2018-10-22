// Copyright 2018 Emre Simsirli

#pragma once

#include "Object.h"
#include "Containers/Map.h"
#include "Containers/Array.h"
#include "Move.h"
#include "PrincipleVariation.generated.h"

UCLASS()
class CHESS_API UPrincipleVariationTable : public UObject
{
    GENERATED_BODY()

    TMap<uint64, FMove> table_; // pos_key, move

public:
    void AddMove(FMove& move, uint64 pos_key);
    TArray<FMove> GetLine(uint32 depth);
    void Clear();

private:
    FMove probe();
};
