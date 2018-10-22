// Copyright 2018 Emre Simsirli

#include "PrincipleVariation.h"
#include "Board.h"
#include "Debug.h"
#include "ChessEngine.h"

void UPrincipleVariationTable::AddMove(FMove& move, const uint64 pos_key)
{
    table_.Add(pos_key, move);
}

TArray<FMove> UPrincipleVariationTable::GetLine(const uint32 depth)
{
    TArray<FMove> arr;
    MAKE_SURE(depth < max_depth);

    auto m = probe();
    while(m != FMove::no_move && arr.Num() < static_cast<int32>(depth)) {
        if(CEngine->move_generator_->DoesMoveExist(m)) {
            CEngine->board_->MakeMove(m);
            arr.Add(m);
        } else break;
        m = probe();
    }

    while(CEngine->board_->ply_ > 0)
        CEngine->board_->TakeMove();
    return arr;
}

void UPrincipleVariationTable::Clear()
{
    table_.Empty();
}

FMove UPrincipleVariationTable::probe()
{
    const auto search = table_.Find(CEngine->board_->pos_key_);
    if(!search)
        return FMove::no_move;
    return *search;
}
