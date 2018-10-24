// Copyright 2018 Emre Simsirli

#pragma once

#include "Object.h"
#include "Runnable.h"
#include "ThreadSafeBool.h"
#include "MoveExplorer.generated.h"

class UMoveGenerator;
class FMove;

UCLASS()
class CHESS_API UMoveExplorer : public UObject
{
    GENERATED_BODY()

    friend UMoveGenerator;
    
public:
    FMove Search() const;

private:
    int32 Evaluate() const;
    int32 AlphaBeta(int32 alpha, int32 beta, uint32 depth) const;
    int32 Quiescence(int32 alpha, int32 beta) const;
};

class FMoveExplorerThread : FRunnable
{
    FRunnableThread* thread_;
    FThreadSafeBool is_killing_;
    FThreadSafeBool is_stopping_search_;

    FEvent* event_;

public:
    FMoveExplorerThread();
    ~FMoveExplorerThread();

    uint32 Run() override;
    void Stop() override;
	void EnsureCompletion();

    void StartSearch();
    void StopSearch();
};
