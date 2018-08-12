// Copyright 2018 Emre Simsirli

#include "Game/ChessGameMode.h"
#include "Game/ChessGameState.h"
#include "Player/ChessPlayer.h"
#include "ChessPlayerController.h"

AChessGameMode::AChessGameMode() {
    DefaultPawnClass = AChessPlayer::StaticClass();
    PlayerControllerClass = AChessPlayerController::StaticClass();
    GameStateClass = AChessGameState::StaticClass();
}
