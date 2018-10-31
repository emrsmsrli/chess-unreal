// Copyright 2018 Emre Simsirli

#include "ChessGameState.h"
#include "UnrealNetwork.h"

void AChessGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    AGameStateBase::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AChessGameState, CurrentSide);
}
