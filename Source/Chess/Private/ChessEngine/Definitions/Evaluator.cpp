// Copyright 2018 Emre Simsirli

#include "Evaluator.h"
#include "Board.h"
#include "Debug.h"
#include "Square.h"
#include "Side.h"
#include "Piece.h"
#include "BoardDefs.h"
#include "PrincipleVariation.h"
#include "Runnable.h"
#include "RunnableThread.h"
#include "ThreadSafeBool.h"
#include "Event.h"

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

class SearchThread : FRunnable
{
    static SearchThread* instance_;

    TEvaluator* ref_;
    FRunnableThread* thread_;
    FThreadSafeBool is_killing_;
    FThreadSafeBool is_giving_up_search_;

    FEvent* event_;

public:
    explicit SearchThread(TEvaluator* ref)
    {
        ref_ = ref;
        thread_ = FRunnableThread::Create(this, TEXT("SearchThread"));
        event_ = FGenericPlatformProcess::GetSynchEventFromPool(false);
        check(thread_);
        check(event_);
    }

    ~SearchThread()
    {
        delete thread_;
        delete event_;
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

    static void initialize(TEvaluator* e) { instance_ = new SearchThread(e); }
    static SearchThread* get() { return instance_; }
};

TEvaluator::TEvaluator(TBoard* ref)
{
    ref_ = ref;
    pv_table_ = new TPrincipleVariationTable(ref_);
    if(ref_->is_multiplayer_)
        search_info_ = new search_info();
    SearchThread::initialize(this);
}

void TEvaluator::search(search_info& info, const TFunction<void(TMove)>& callback) const
{
    pv_table_->empty();
    ref_->ply_ = 0;
    SearchThread::get()->start_search();

    /*best_move_ = Async<TMove>(EAsyncExecution::ThreadPool, [&]() -> TMove
    {
        return TMove::no_move;
    }, [&]()
    {
        callback(best_move_.Get());
    });*/

    //~ iterative deepening
    for(auto depth = 1; depth <= info.depth; ++depth) {
        const auto best_score = alpha_beta(-INFINITE, INFINITE, depth, info, true);

        // out of time check

        const auto pvmoves = pv_table_->get_line(depth);
        const auto best_move = pvmoves[0];

        UE_LOG(LogTemp, Log, TEXT("depth %d, score %d, move: %s, nodes %ld"), depth,
            best_score, *best_move.ToString(), info.nodes);

        FString str = "pv";
        for(auto& move : pvmoves) {
            str += " " + move.ToString();
        }

        UE_LOG(LogTemp, Log, TEXT("%s"), *str);
        UE_LOG(LogTemp, Log, TEXT("Ordering %.2f"), info.fh == 0 ? 0 : info.fhf / info.fh);
    }
}

int32 TEvaluator::evaluate() const
{
    int32 score = ref_->material_score_[ESide::white] - ref_->material_score_[ESide::black];

    /*~ white pawn ~*/
    for(auto sq : ref_->piece_locations_[EPieceType::wp]) {
        MAKE_SURE(Verification::IsSquareOnBoard(sq));
        score += PawnTable[ESquare::Sq64(sq)];
    }

    /*~ black pawn ~*/
    for(auto sq : ref_->piece_locations_[EPieceType::bp]) {
        MAKE_SURE(Verification::IsSquareOnBoard(sq));
        score -= PawnTable[Mirror[ESquare::Sq64(sq)]];
    }

    /*~ white knight ~*/

    for(auto sq : ref_->piece_locations_[EPieceType::wn]) {
        MAKE_SURE(Verification::IsSquareOnBoard(sq));
        score += KnightTable[ESquare::Sq64(sq)];
    }

    /*~ black knight ~*/
    for(auto sq : ref_->piece_locations_[EPieceType::bn]) {
        MAKE_SURE(Verification::IsSquareOnBoard(sq));
        score -= KnightTable[Mirror[ESquare::Sq64(sq)]];
    }

    /*~ white bishop ~*/
    for(auto sq : ref_->piece_locations_[EPieceType::wb]) {
        MAKE_SURE(Verification::IsSquareOnBoard(sq));
        score += BishopTable[ESquare::Sq64(sq)];
    }

    /*~ black bishop ~*/
    for(auto sq : ref_->piece_locations_[EPieceType::bb]) {
        MAKE_SURE(Verification::IsSquareOnBoard(sq));
        score -= BishopTable[Mirror[ESquare::Sq64(sq)]];
    }

    /*~ white rook ~*/
    for(auto sq : ref_->piece_locations_[EPieceType::wr]) {
        MAKE_SURE(Verification::IsSquareOnBoard(sq));
        score += RookTable[ESquare::Sq64(sq)];
    }

    /*~ black rook ~*/
    for(auto sq : ref_->piece_locations_[EPieceType::br]) {
        MAKE_SURE(Verification::IsSquareOnBoard(sq));
        score -= RookTable[Mirror[ESquare::Sq64(sq)]];
    }

    return ref_->side_ == ESide::white ? score : -score;
}

int32 TEvaluator::alpha_beta(int32 alpha, const int32 beta, const uint32 depth,
                             search_info& info, const bool do_null) const
{
    MAKE_SURE(is_valid());

    info.nodes++;

    if(depth == 0) {
        return evaluate();
    }

    if(ref_->fifty_move_counter_ >= 100 || ref_->has_repetition())
        return 0; // draw

    if(ref_->ply_ > max_depth - 1)
        return evaluate();

    uint32 legal = 0;
    const auto old_alpha = alpha;
    auto moves = ref_->generate_moves();
    auto best_move = TMove::no_move;

    moves.Sort([](const TMove& lhs, const TMove& rhs) -> bool
    {
        return lhs.score() > rhs.score();
    });

    for(auto& move : moves) {
        if(!ref_->make_move(move))
            continue;

        legal++;
        const auto score = -alpha_beta(-beta, -alpha, depth - 1, info, do_null);
        ref_->take_move();

        if(score > alpha) {
            if(score >= beta) {
                if(legal == 1)
                    info.fhf++;
                info.fh++;

                if(!move.is_captured())
                    info.add_killer(ref_->ply_, move);

                return beta;
            }
            alpha = score;
            best_move = move;

            if(!move.is_captured()) {
                info.history[ref_->b_[best_move.from()]][best_move.to()] += depth;
            }
        }
    }

    if(legal == 0) {
        if(ref_->is_attacked(ref_->king_sq_[ref_->side_], ref_->side_ ^ 1))
            return -MATE + ref_->ply_; // mate
        return 0; // stalemate and draw
    }

    if(alpha != old_alpha)
        pv_table_->add_move(best_move, ref_->pos_key_);

    return alpha;
}

int32 TEvaluator::quiescence(int32 alpha, int32 beta, search_info& info) const
{
    return 0;
}
