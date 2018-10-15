// Copyright 2018 Emre Simsirli

#pragma once

#include "Containers/Map.h"
#include "Containers/Array.h"
#include "Move.h"

class TPrincipleVariationTable
{
    class TBoard* b_;
    TMap<uint64, TMove> table_; // pos_key, move

public:
    explicit TPrincipleVariationTable(class TBoard* b_ref);

    void add_move(TMove& move, uint64 pos_key);
    TArray<TMove> get_line(uint32 depth);
    void empty();

private:
    TMove probe();
};
