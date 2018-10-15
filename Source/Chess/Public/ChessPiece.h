// Copyright 2018 Emre Simsirli

#pragma once

#include "DestructibleActor.h"
#include "Side.h"
#include "ChessPiece.generated.h"

UCLASS()
class CHESS_API AChessPiece : public ADestructibleActor
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
    TEnumAsByte<ESide::Type> Side;

    AChessPiece();

protected:
    void BeginPlay() override;

public:
    void Tick(float DeltaTime) override;
};
