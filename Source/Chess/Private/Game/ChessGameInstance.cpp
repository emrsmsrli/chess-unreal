// Copyright 2018 Emre Simsirli

#include "ChessGameInstance.h"
#include "Widget.h"
//#include ""

UChessGameInstance::UChessGameInstance() {
    State = EState::START;
}

bool UChessGameInstance::HostSession(FName SessionName, bool IsLAN) {
    //IOnlineSubsystem
}
