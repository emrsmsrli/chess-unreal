// Copyright 2018 Emre Simsirli

#include "ChessGameSession.h"
#include "ChessPlayerController.h"

AChessGameSession::AChessGameSession()
{
    connected_players_ = TArray<APlayerController*>();
}

void AChessGameSession::PostLogin(APlayerController* NewPlayer)
{
    connected_players_.AddUnique(NewPlayer);
}

uint32 AChessGameSession::GetConnectedPlayerNum()
{
    return connected_players_.Num();
}
