// Copyright 2018 Emre Simsirli

#include "ChessEngine.h"
#include "Bitboard.h"
#include "Board.h"
#include "PosKey.h"
#include "Square.h"
#include "Debug.h"
#include "BoardDefs.h"

UChessEngine* CEngine = nullptr;

void UChessEngine::Initialize()
{
    CEngine = NewObject<UChessEngine>();
}

UChessEngine::UChessEngine()
{
    ESquare::Initialize();
    FBitboard::Initialize();
    PosKey::Initialize();
    UMoveGenerator::Initialize();

    board_ = NewObject<UBoard>();   
    move_generator_ = NewObject<UMoveGenerator>();
    evaluator_ = NewObject<UEvaluator>();
    pv_table_ = NewObject<UPrincipleVariationTable>();
    search_info = new FSearchInfo();
}

void UChessEngine::Search() const
{
    evaluator_->Search();
}

#ifdef DEBUG
void UChessEngine::Set(FString& fen) const
{
    board_->Set(fen);
}

void UChessEngine::Perft(const int32 depth, int64* leaf_nodes) const
{
    if(depth == 0) {
        ++*leaf_nodes;
        return;
    }

    auto moves = move_generator_->GenerateMoves();
    for(auto i = 0; i < moves.Num(); i++) {
        if(!board_->MakeMove(moves[i]))
            continue;

        Perft(depth - 1, leaf_nodes);
        board_->TakeMove();
    }
}

FString UChessEngine::Perft(const int32 depth) const
{
    int64 leaf_nodes = 0;
    auto moves = move_generator_->GenerateMoves();
    for(auto& m : moves) {
        if(!board_->MakeMove(m))
            continue;
        Perft(depth - 1, &leaf_nodes);
        board_->TakeMove();
    }
    return FString::Printf(TEXT(" ;D%d %d"), depth, leaf_nodes);
}
#endif
