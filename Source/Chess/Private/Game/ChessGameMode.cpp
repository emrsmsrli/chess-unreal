// Copyright 2018 Emre Simsirli

#include "ChessGameMode.h"
#include "ChessGameState.h"
#include "ChessPlayerController.h"
#include "ChessGameSession.h"
#include "Player/ChessPlayer.h"
#include "Engine/Engine.h"
#include "ChessPlayerState.h"

AChessGameMode::AChessGameMode() {
    DefaultPawnClass = AChessPlayer::StaticClass();
    PlayerControllerClass = AChessPlayerController::StaticClass();
    GameStateClass = AChessGameState::StaticClass();
    PlayerStateClass = AChessPlayerState::StaticClass();
    GameSessionClass = AChessGameSession::StaticClass();
}

void AChessGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) {
    AGameModeBase::InitGame(MapName, Options, ErrorMessage);
    // todo initialize chess engine stuff
}

void AChessGameMode::PostLogin(APlayerController* NewPlayer) {
    AGameModeBase::PostLogin(NewPlayer);
    if(auto* state = Cast<AChessPlayerState>(NewPlayer->PlayerState))
        UE_LOG(LogTemp, Log, TEXT("logged in, name: %s"), *state->Name);
}
