// Copyright 2018 Emre Simsirli

#pragma once

#include "GameFramework/Actor.h"
#include "Move.h"
#include "MoveHintPlane.generated.h"

UCLASS()
class CHESS_API AMoveHintPlane : public AActor
{
    GENERATED_BODY()

    const FMoveData* move_;
	
public:	
    AMoveHintPlane();
    void SetMove(const FMoveData* m);
    const FMoveData* GetMove() const;

    void SetLocation(float x, float y);
};
