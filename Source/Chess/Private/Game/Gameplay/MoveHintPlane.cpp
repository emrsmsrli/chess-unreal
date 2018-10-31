// Copyright 2018 Emre Simsirli

#include "MoveHintPlane.h"

AMoveHintPlane::AMoveHintPlane()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AMoveHintPlane::SetMove(const FMoveData* m)
{
    move_ = m;
}

const FMoveData* AMoveHintPlane::GetMove() const
{
    return move_;
}

void AMoveHintPlane::SetLocation(const float x, const float y)
{
    SetActorLocation(FVector(x, y, .1f));
}
