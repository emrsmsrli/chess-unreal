// Copyright 2018 Emre Simsirli

#pragma once

#include "GameFramework/GameSession.h"
#include "ChessGameSession.generated.h"

UCLASS()
class CHESS_API AChessGameSession : public AGameSession
{
    GENERATED_BODY()

    TArray<class APlayerController*> connected_players_;

public:
    AChessGameSession();
    void PostLogin(APlayerController* NewPlayer) override;
    uint32 GetConnectedPlayerNum();
};
