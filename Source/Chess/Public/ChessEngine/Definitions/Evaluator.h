// Copyright 2018 Emre Simsirli

#pragma once

#include "Object.h"
#include "Evaluator.generated.h"

class UMoveGenerator;

UCLASS()
class CHESS_API UEvaluator : public UObject
{
    GENERATED_BODY()

    friend UMoveGenerator;
    
public:
    void Search() const;

private:
    int32 Evaluate() const;
    int32 AlphaBeta(int32 alpha, int32 beta, uint32 depth, bool do_null) const;
    int32 Quiescence(int32 alpha, int32 beta) const;
};
