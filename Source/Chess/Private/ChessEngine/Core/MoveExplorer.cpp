// Copyright 2018 Emre Simsirli

#include "MoveExplorer.h"
#include "Board.h"
#include "Debug.h"
#include "Square.h"
#include "Side.h"
#include "PieceInfo.h"
#include "Search.h"
#include "PrincipleVariation.h"
#include "RunnableThread.h"
#include "ThreadSafeBool.h"
#include "Event.h"
#include "ChessEngine.h"
#include "Async.h"
#include "Util/Log.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

#ifdef DEBUG
#include "Verify.h"
#endif

#define INFINITE 30000
#define MATE 29000

#define PV_MOVE_SCORE 2000000

namespace
{
    const int32 PawnTable[64] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        10, 10, 0, -10, -10, 0, 10, 10,
        5, 0, 0, 5, 5, 0, 0, 5,
        0, 0, 10, 20, 20, 10, 0, 0,
        5, 5, 5, 10, 10, 5, 5, 5,
        10, 10, 10, 20, 20, 10, 10, 10,
        20, 20, 20, 30, 30, 20, 20, 20,
        0, 0, 0, 0, 0, 0, 0, 0
    };

    const int32 KnightTable[64] = {
        0, -10, 0, 0, 0, 0, -10, 0,
        0, 0, 0, 5, 5, 0, 0, 0,
        0, 0, 10, 10, 10, 10, 0, 0,
        0, 0, 10, 20, 20, 10, 5, 0,
        5, 10, 15, 20, 20, 15, 10, 5,
        5, 10, 10, 20, 20, 10, 10, 5,
        0, 0, 5, 10, 10, 5, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
    };

    const int32 BishopTable[64] = {
        0, 0, -10, 0, 0, -10, 0, 0,
        0, 0, 0, 10, 10, 0, 0, 0,
        0, 0, 10, 15, 15, 10, 0, 0,
        0, 10, 15, 20, 20, 15, 10, 0,
        0, 10, 15, 20, 20, 15, 10, 0,
        0, 0, 10, 15, 15, 10, 0, 0,
        0, 0, 0, 10, 10, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
    };

    const int32 RookTable[64] = {
        0, 0, 5, 10, 10, 5, 0, 0,
        0, 0, 5, 10, 10, 5, 0, 0,
        0, 0, 5, 10, 10, 5, 0, 0,
        0, 0, 5, 10, 10, 5, 0, 0,
        0, 0, 5, 10, 10, 5, 0, 0,
        0, 0, 5, 10, 10, 5, 0, 0,
        25, 25, 25, 25, 25, 25, 25, 25,
        0, 0, 5, 10, 10, 5, 0, 0
    };

    const uint32 Mirror[64] = {
        56, 57, 58, 59, 60, 61, 62, 63,
        48, 49, 50, 51, 52, 53, 54, 55,
        40, 41, 42, 43, 44, 45, 46, 47,
        32, 33, 34, 35, 36, 37, 38, 39,
        24, 25, 26, 27, 28, 29, 30, 31,
        16, 17, 18, 19, 20, 21, 22, 23,
        8, 9, 10, 11, 12, 13, 14, 15,
        0, 1, 2, 3, 4, 5, 6, 7
    };
}

FMove UMoveExplorer::Search() const
{
    LOGI("beginning with depth: %d, time set: %d, null cut: %d",
        CEngine->SearchParams.Depth,
        CEngine->SearchParams.TimeSet,
        CEngine->SearchParams.UseNullCut);
    CEngine->SearchInfo->Clear();
    CEngine->pv_table_->Clear();
    CEngine->board_->ply_ = 0;

    CEngine->SearchInfo->StartTime = UGameplayStatics::GetRealTimeSeconds(GetWorld());
    CEngine->SearchInfo->StopTimeSet = CEngine->SearchInfo->StartTime + CEngine->SearchParams.TimeSet;

    auto best_move = FMove::no_move;

    //~ iterative deepening
    for(auto depth = 1; depth <= CEngine->SearchParams.Depth; ++depth) {
        const auto best_score = AlphaBeta(-INFINITE, INFINITE, depth);

        if(CEngine->SearchInfo->bStopRequested)
            break;

        const auto pvmoves = CEngine->pv_table_->GetLine(depth);
        best_move = pvmoves[0];

#ifdef DEBUG
        LOGI("depth %d, score %d, move: %s, nodes %ld",
			depth, best_score, *best_move.ToString(), 
			CEngine->SearchInfo->TotalVisitedNodes);

        FString str = "pv";
        for(auto& move : pvmoves) {
            str += " " + move.ToString();
        }

        LOGI("%s", *str);
        LOGI("Ordering %.2f", CEngine->SearchInfo->F_H == 0 ? 0 :
            CEngine->SearchInfo->F_H_F / CEngine->SearchInfo->F_H);
#endif
    }

    CEngine->SearchInfo->StopTimeActual = UGameplayStatics::GetRealTimeSeconds(GetWorld());
    LOGI("best move found: %s, took %f secs, actual-set diff %f secs", *best_move.ToString(),
        CEngine->SearchInfo->StopTimeActual - CEngine->SearchInfo->StartTime,
        CEngine->SearchInfo->StopTimeActual - CEngine->SearchInfo->StopTimeSet);

    return best_move;
}

int32 UMoveExplorer::Evaluate() const
{
    auto* board = CEngine->board_;
    int32 score = board->material_score_[ESide::white] - board->material_score_[ESide::black];

    /*~ white pawn ~*/
    for(auto sq : board->piece_locations_[EPieceType::wp]) {
        MAKE_SURE(Verification::IsSquareOnBoard(sq));
        score += PawnTable[ESquare::Sq64(sq)];
    }

    /*~ black pawn ~*/
    for(auto sq : board->piece_locations_[EPieceType::bp]) {
        MAKE_SURE(Verification::IsSquareOnBoard(sq));
        score -= PawnTable[Mirror[ESquare::Sq64(sq)]];
    }

    /*~ white knight ~*/

    for(auto sq : board->piece_locations_[EPieceType::wn]) {
        MAKE_SURE(Verification::IsSquareOnBoard(sq));
        score += KnightTable[ESquare::Sq64(sq)];
    }

    /*~ black knight ~*/
    for(auto sq : board->piece_locations_[EPieceType::bn]) {
        MAKE_SURE(Verification::IsSquareOnBoard(sq));
        score -= KnightTable[Mirror[ESquare::Sq64(sq)]];
    }

    /*~ white bishop ~*/
    for(auto sq : board->piece_locations_[EPieceType::wb]) {
        MAKE_SURE(Verification::IsSquareOnBoard(sq));
        score += BishopTable[ESquare::Sq64(sq)];
    }

    /*~ black bishop ~*/
    for(auto sq : board->piece_locations_[EPieceType::bb]) {
        MAKE_SURE(Verification::IsSquareOnBoard(sq));
        score -= BishopTable[Mirror[ESquare::Sq64(sq)]];
    }

    /*~ white rook ~*/
    for(auto sq : board->piece_locations_[EPieceType::wr]) {
        MAKE_SURE(Verification::IsSquareOnBoard(sq));
        score += RookTable[ESquare::Sq64(sq)];
    }

    /*~ black rook ~*/
    for(auto sq : board->piece_locations_[EPieceType::br]) {
        MAKE_SURE(Verification::IsSquareOnBoard(sq));
        score -= RookTable[Mirror[ESquare::Sq64(sq)]];
    }

    return board->side_ == ESide::white ? score : -score;
}

int32 UMoveExplorer::AlphaBeta(int32 alpha, const int32 beta, const uint32 depth) const
{
    auto* board = CEngine->board_;

    if(depth == 0)
        return Quiescence(alpha, beta);

    if((CEngine->SearchInfo->TotalVisitedNodes & 2047) == 0)
        CheckTimeIsUp();

    CEngine->SearchInfo->TotalVisitedNodes++;

    if(board->fifty_move_counter_ >= 100 || board->HasRepetition())
        return 0; // draw

    if(board->ply_ > max_depth - 1)
        return Evaluate();

    uint32 legal = 0;
    const auto old_alpha = alpha;
    auto moves = CEngine->move_generator_->GenerateMoves();
    auto best_move = FMove::no_move;

    // pv move heuristic
    const auto pv_move = CEngine->pv_table_->probe();
    if(pv_move != FMove::no_move) {
        const auto m = moves.FindByPredicate([&pv_move](const FMove& mm) -> bool
        {
            return mm == pv_move;
        });
        if(m) m->SetScore(PV_MOVE_SCORE);
    }

    moves.Sort([](const FMove& lhs, const FMove& rhs) -> bool
    {
        return lhs.GetScore() > rhs.GetScore();
    });

    for(auto& move : moves) {
        if(!board->MakeMove(move))
            continue;

        legal++;
        const auto score = -AlphaBeta(-beta, -alpha, depth - 1);
        board->TakeMove();

        if(CEngine->SearchInfo->bStopRequested)
            return 0;

        if(score > alpha) {
            if(score >= beta) {
#ifdef DEBUG
                if(legal == 1)
                    CEngine->SearchInfo->F_H_F++;
                CEngine->SearchInfo->F_H++;
#endif

                if(!move.IsCaptured())
                    CEngine->SearchInfo->AddKiller(board->ply_, move);

                return beta;
            }
            alpha = score;
            best_move = move;

            if(!move.IsCaptured())
                CEngine->SearchInfo->AddHistory(board->b_[best_move.From()], best_move.To(), depth);
        }
    }

    if(legal == 0) {
        if(board->IsAttacked(board->king_sq_[board->side_], board->side_ ^ 1))
            return -MATE + board->ply_; // mate
        return 0; // stalemate and draw
    }

    if(alpha != old_alpha)
        CEngine->pv_table_->AddMove(best_move, board->pos_key_);

    return alpha;
}

int32 UMoveExplorer::Quiescence(int32 alpha, const int32 beta) const
{
    auto* board = CEngine->board_;
    MAKE_SURE(board->IsOk());

    if((CEngine->SearchInfo->TotalVisitedNodes & 2047) == 0)
        CheckTimeIsUp();

    CEngine->SearchInfo->TotalVisitedNodes++;

    if(board->fifty_move_counter_ >= 100 || board->HasRepetition())
        return 0; // draw

    if(board->ply_ > max_depth - 1)
        return Evaluate();

    const auto stand_pat = Evaluate();
    if(stand_pat >= beta)
        return beta;

    if(stand_pat > alpha)
        alpha = stand_pat;
    
#ifdef DEBUG
    uint32 legal = 0;
#endif
    const auto old_alpha = alpha;
    auto best_move = FMove::no_move;
    auto moves = CEngine->move_generator_->GenerateMoves();

    // only process captured moves
    moves = moves.FilterByPredicate([](const FMove& move) -> bool
    {
        return move.IsCaptured();
    });

    moves.Sort([](const FMove& lhs, const FMove& rhs) -> bool
    {
        return lhs.GetScore() > rhs.GetScore();
    });

    for(auto& move : moves) {
        if(!board->MakeMove(move))
            continue;
        
#ifdef DEBUG
        legal++;
#endif
        const auto score = -Quiescence(-beta, -alpha);
        board->TakeMove();

        if(CEngine->SearchInfo->bStopRequested)
            return 0;

        if(score > alpha) {
            if(score >= beta) {
#ifdef DEBUG
                if(legal == 1)
                    CEngine->SearchInfo->F_H_F++;
                CEngine->SearchInfo->F_H++;
#endif

                return beta;
            }
            alpha = score;
            best_move = move;
        }
    }

    if(alpha != old_alpha)
        CEngine->pv_table_->AddMove(best_move, CEngine->board_->pos_key_);

    return alpha;
}

void UMoveExplorer::CheckTimeIsUp() const
{
    if(!CEngine->SearchParams.TimeSet)
        return;
    const auto end_time = CEngine->SearchInfo->StartTime + CEngine->SearchParams.TimeSet;
    if(end_time >= UGameplayStatics::GetRealTimeSeconds(GetWorld()))
        CEngine->SearchInfo->bStopRequested = true;
}

FMoveExplorerThread::FMoveExplorerThread()
{
    LOGI("thread starting");
    event_ = FGenericPlatformProcess::GetSynchEventFromPool(false);
    check(event_);
    thread_ = FRunnableThread::Create(this, TEXT("SearchThread"));
    check(thread_);
}

FMoveExplorerThread::~FMoveExplorerThread()
{
    if(event_) {
        FGenericPlatformProcess::ReturnSynchEventToPool(event_);
        event_ = nullptr;
    }

    delete thread_;
}

uint32 FMoveExplorerThread::Run()
{
    // initial wait so that engine does not go wild
    // and start searching before game start
    event_->Wait();

    while(!is_killing_) {
        if(!is_stopping_search_) {
            FPlatformProcess::Sleep(0.01);

            const auto best_move = CEngine->move_explorer_->Search();
            AsyncTask(ENamedThreads::GameThread, [best_move]() -> void
            {
                // should be bound by the caller until this point
                CEngine->MoveFoundDelegate.Execute(best_move);
            });
            StopSearch();
        } else {
            event_->Wait();

            if(is_killing_)
                break;
        }
    }

    LOGI("thread exiting");
    return 0;
}

void FMoveExplorerThread::Stop()
{
    DoStop();
    thread_->WaitForCompletion();
}

void FMoveExplorerThread::DoStop()
{
    is_killing_ = true;
    StartSearch(); // breaks out of the loop
}

void FMoveExplorerThread::StartSearch()
{
    is_stopping_search_ = false;
    event_->Trigger();
}

void FMoveExplorerThread::StopSearch()
{
    is_stopping_search_ = true;
}
