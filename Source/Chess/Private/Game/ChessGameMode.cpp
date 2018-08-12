// Copyright 2018 Emre Simsirli

#include "Game/ChessGameMode.h"
#include "Game/ChessGameState.h"
#include "Player/PlayerPawn.h"
#include "PlayerPawnController.h"

AChessGameMode::AChessGameMode() {
    DefaultPawnClass = APlayerPawn::StaticClass();
    PlayerControllerClass = APlayerPawnController::StaticClass();
    GameStateClass = AChessGameState::StaticClass();
}
