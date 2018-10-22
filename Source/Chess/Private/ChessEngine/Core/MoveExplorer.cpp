// Copyright 2018 Emre Simsirli

#include "MoveExplorer.h"
#include "Board.h"
#include "Debug.h"
#include "Square.h"
#include "Side.h"
#include "PieceInfo.h"
#include "Search.h"
#include "PrincipleVariation.h"
#include "Runnable.h"
#include "RunnableThread.h"
#include "ThreadSafeBool.h"
#include "Event.h"
#include "ChessEngine.h"
#include "Verify.h"

#define INFINITE 30000
#define MATE 29000

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

class FMoveSearchThread : FRunnable
{
    UMoveExplorer* ref_;
    FRunnableThread* thread_;
    FThreadSafeBool is_killing_;
    FThreadSafeBool is_giving_up_search_;

    FEvent* event_;

public:
    FMoveSearchThread()
    {
        thread_ = FRunnableThread::Create(this, TEXT("SearchThread"));
        event_ = FGenericPlatformProcess::GetSynchEventFromPool(false);
        check(thread_);
        check(event_);
    }

    uint32 Run() override
    {
        while(!is_killing_) {
            if(!is_giving_up_search_) {
                FPlatformProcess::Sleep(0.1);
                // todo
            } else {
                event_->Wait();

                if(is_killing_)
                    break;
            }
        }

        return 0;
    }

    void Stop() override
    {
        is_killing_ = true;
        start_search(); // breaks out of the loop
    }

    void start_search()
    {
        is_giving_up_search_ = false;
        event_->Trigger();
    }

    void give_up_search() { is_giving_up_search_ = true; }
};

//auto* search_thread = new FMoveSearchThread();

void UMoveExplorer::Search() const
{
    CEngine->SearchInfo->Clear();
    CEngine->pv_table_->Clear();
    CEngine->board_->ply_ = 0;

    //search_thread->start_search();

    /*best_move_ = Async<TMove>(EAsyncExecution::ThreadPool, [&]() -> TMove
    {
        return TMove::no_move;
    }, [&]()
    {
        callback(best_move_.Get());
    });*/

    //~ iterative deepening
    for(auto depth = 1; depth <= CEngine->SearchParams->Depth; ++depth) {
        const auto best_score = AlphaBeta(-INFINITE, INFINITE, depth);

        // out of time check

        const auto pvmoves = CEngine->pv_table_->GetLine(depth);
        const auto best_move = pvmoves[0];

        UE_LOG(LogTemp, Log, TEXT("depth %d, score %d, move: %s, nodes %ld"), depth,
            best_score, *best_move.ToString(), CEngine->SearchInfo->TotalVisitedNodes);

        FString str = "pv";
        for(auto& move : pvmoves) {
            str += " " + move.ToString();
        }

        UE_LOG(LogTemp, Log, TEXT("%s"), *str);
        UE_LOG(LogTemp, Log, TEXT("Ordering %.2f"), CEngine->SearchInfo->F_H == 0 ? 0 : 
			CEngine->SearchInfo->F_H_F / CEngine->SearchInfo->F_H);
    }
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

    CEngine->SearchInfo->TotalVisitedNodes++;

    if(depth == 0) {
        return Evaluate();
    }

    if(board->fifty_move_counter_ >= 100 || board->HasRepetition())
        return 0; // draw

    if(board->ply_ > max_depth - 1)
        return Evaluate();

    uint32 legal = 0;
    const auto old_alpha = alpha;
    auto moves = CEngine->move_generator_->GenerateMoves();
    auto best_move = FMove::no_move;

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

        if(score > alpha) {
            if(score >= beta) {
                if(legal == 1)
                    CEngine->SearchInfo->F_H_F++;
                CEngine->SearchInfo->F_H++;

                if(!move.IsCaptured())
                    CEngine->SearchInfo->AddKiller(board->ply_, move);

                return beta;
            }
            alpha = score;
            best_move = move;

            if(!move.IsCaptured()) {
                CEngine->SearchInfo->AddHistory(board->b_[best_move.From()], best_move.To(), depth);
            }
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

int32 UMoveExplorer::Quiescence(int32 alpha, int32 beta) const
{
    return 0;
}
