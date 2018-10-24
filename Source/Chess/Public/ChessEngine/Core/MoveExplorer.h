// Copyright 2018 Emre Simsirli

#pragma once

#include "Object.h"
#include "MoveExplorer.generated.h"
#include "Runnable.h"
#include "ThreadSafeBool.h"

class UMoveGenerator;

UCLASS()
class CHESS_API UMoveExplorer : public UObject
{
    GENERATED_BODY()

    friend UMoveGenerator;
    
public:
    void Search() const;

private:
    int32 Evaluate() const;
    int32 AlphaBeta(int32 alpha, int32 beta, uint32 depth) const;
    int32 Quiescence(int32 alpha, int32 beta) const;
};

class FMoveExplorerThread : FRunnable
{
    FRunnableThread* thread_;
    FThreadSafeBool is_killing_;
    FThreadSafeBool is_giving_up_search_;

    FEvent* event_;

public:
    FMoveExplorerThread();

    uint32 Run() override;
    void Stop() override;

    void StartSearch();
    void GiveUpSearch();
};
