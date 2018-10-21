// Copyright 2018 Emre Simsirli

#pragma once

#include "Object.h"
#include "Platform.h"
#include "Evaluator.generated.h"

UCLASS()
class UEvaluator : public UObject
{
    GENERATED_BODY()

    friend class UMoveGenerator;
    
public:
    void Search() const;

private:
    int32 Evaluate() const;
    int32 AlphaBeta(int32 alpha, int32 beta, uint32 depth, bool do_null) const;
    int32 Quiescence(int32 alpha, int32 beta) const;
};
