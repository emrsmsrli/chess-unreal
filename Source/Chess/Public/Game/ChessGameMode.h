// Copyright 2018 Emre Simsirli

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ChessGameMode.generated.h"

UCLASS()
class CHESS_API AChessGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AChessGameMode();
    void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
    void PostLogin(APlayerController* NewPlayer) override;
};
