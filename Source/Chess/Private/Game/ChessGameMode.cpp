// Copyright 2018 Emre Simsirli

#include "ChessGameMode.h"
#include "ChessGameState.h"
#include "ChessPlayerController.h"
#include "ChessGameSession.h"
#include "Player/ChessPlayer.h"

AChessGameMode::AChessGameMode() {
    DefaultPawnClass = AChessPlayer::StaticClass();
    PlayerControllerClass = AChessPlayerController::StaticClass();
    GameStateClass = AChessGameState::StaticClass();
    GameSessionClass = AChessGameSession::StaticClass();
}
