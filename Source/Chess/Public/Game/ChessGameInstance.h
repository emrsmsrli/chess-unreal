// Copyright 2018 Emre Simsirli

#pragma once

#include "Engine/GameInstance.h"
#include "State.h"
#include "ChessGameInstance.generated.h"

UCLASS()
class CHESS_API UChessGameInstance : public UGameInstance {
    GENERATED_BODY()
        
public:
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Gameplay")
    TEnumAsByte<EState::Type> State;

    UChessGameInstance();
    bool HostSession(FName SessionName, bool IsLAN);
};
