// Copyright 2018 Emre Simsirli

#include "PrincipleVariation.h"
#include "Board.h"
#include "Debug.h"

TPrincipleVariationTable::TPrincipleVariationTable(TBoard* b_ref)
{
    b_ = b_ref;
}

void TPrincipleVariationTable::add_move(TMove& move, const uint64 pos_key)
{
    table_.Add(pos_key, move);
}

TArray<TMove> TPrincipleVariationTable::get_line(const uint32 depth)
{
    TArray<TMove> arr;
    MAKE_SURE(depth < max_depth);

    auto m = probe();
    while(m != TMove::no_move && arr.Num() < static_cast<int32>(depth)) {
        if(b_->move_exists(m)) {
            b_->make_move(m);
            arr.Add(m);
        } else break;
        m = probe();
    }

    while(b_->ply_ > 0)
        b_->take_move();
    return arr;
}

void TPrincipleVariationTable::empty()
{
    table_.Empty();
}

TMove TPrincipleVariationTable::probe()
{
    const auto search = table_.Find(b_->pos_key_);
    if(!search)
        return TMove::no_move;
    return *search;
}
