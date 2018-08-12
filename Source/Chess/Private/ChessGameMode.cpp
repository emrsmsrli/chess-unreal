// Copyright 2018 Emre Simsirli

#include "ChessGameMode.h"
#include "Player/PlayerPawn.h"
#include "PlayerPawnController.h"

AChessGameMode::AChessGameMode() {
    DefaultPawnClass = APlayerPawn::StaticClass();
    PlayerControllerClass = APlayerPawnController::StaticClass();
}
