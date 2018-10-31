// Copyright 2018 Emre Simsirli

#pragma once

#include "GameFramework/GameStateBase.h"
#include "Side.h"
#include "ChessGameState.generated.h"

UCLASS()
class CHESS_API AChessGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    UPROPERTY(Replicated)
    TEnumAsByte<ESide::Type> CurrentSide;

    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
