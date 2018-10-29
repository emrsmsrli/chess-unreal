// Copyright 2018 Emre Simsirli

#include "ChessEngine.h"
#include "Bitboard.h"
#include "Board.h"
#include "PosKey.h"
#include "Square.h"
#include "Debug.h"
#include "Search.h"
#include "Side.h"
#include "Util/Log.h"

UChessEngine* CEngine = nullptr;

void UChessEngine::Initialize()
{
    LOGI("engine initialization starting");
    CEngine = NewObject<UChessEngine>();
    LOGI("engine initialization complete");
}

void UChessEngine::Shutdown()
{
    CEngine->move_explorer_thread_->Stop();
    delete CEngine->move_explorer_thread_;
    delete CEngine->SearchInfo;
    CEngine = nullptr;
}

void UChessEngine::CheckGameOver() const
{
    if(board_->DoesViolateFiftyMoveRule())
        UpdateGameStateDelegate.Execute(EGameState::draw, EGameOverReason::fifty_move);
	else if(board_->HasTrifoldRepetition())
        UpdateGameStateDelegate.Execute(EGameState::draw, EGameOverReason::trifold_repetition);
	else if(board_->IsDrawByMaterial())
        UpdateGameStateDelegate.Execute(EGameState::draw, EGameOverReason::insufficent_material);
	else {
	    const auto moves = move_generator_->GenerateMoves();
        const auto legal_move = moves.FindByPredicate([&](const FMove& m) -> bool
        {
            return board_->MakeMove(m);
        });

        if(legal_move) {
            board_->TakeMove();
            UpdateGameStateDelegate.Execute(EGameState::not_over, EGameOverReason::none);
            return;
		}

        if(board_->IsInCheck()) {
            switch(board_->GetSide()) {
			case ESide::white:
                UpdateGameStateDelegate.Execute(EGameState::mate, EGameOverReason::mate_black);
                break;
			case ESide::black:
                UpdateGameStateDelegate.Execute(EGameState::mate, EGameOverReason::mate_white);
                break;
			default:break;
            }
        } else {
            UpdateGameStateDelegate.Execute(EGameState::draw, EGameOverReason::stalemate);
		}
	}
}

UChessEngine::UChessEngine()
{
    ESquare::Initialize();
    FBitboard::Initialize();
    PosKey::Initialize();
    UMoveGenerator::Initialize();

    board_ = NewObject<UBoard>();   
    move_generator_ = NewObject<UMoveGenerator>();
    move_explorer_ = NewObject<UMoveExplorer>();
    pv_table_ = NewObject<UPrincipleVariationTable>();
    move_explorer_thread_ = new FMoveExplorerThread();
    SearchInfo = new FSearchInfo();
}

void UChessEngine::Set(FString& fen) const
{
    board_->Set(fen);
}

void UChessEngine::MakeMove(FMove& move) const
{
    board_->MakeMove(move);
    CheckGameOver();
}

void UChessEngine::TakeMove() const
{
    board_->TakeMove();
}

TArray<FMove> UChessEngine::GenerateMoves(const uint32 sq) const
{
    return move_generator_->GenerateMoves(sq);
}

void UChessEngine::Search() const
{
    move_explorer_thread_->StartSearch();
}

#ifdef DEBUG
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
