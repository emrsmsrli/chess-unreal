// Copyright 2018 Emre Simsirli

#include "Online/ChessSessionResultHelper.h"

FString UChessSessionResultHelper::GetName(const FChessSessionSearchResult& search) {
    FString name;
    search.Result.Session.SessionSettings.Get(KEY_LOBBY_NAME, name);
    return name;
}

int32 UChessSessionResultHelper::GetPing(const FChessSessionSearchResult& search) {
    return search.Result.PingInMs;
}

uint8 UChessSessionResultHelper::GetCurrentPlayers(const FChessSessionSearchResult& search) {
    return search.Result.Session.SessionSettings.NumPublicConnections - search.Result.Session.NumOpenPublicConnections;
}

uint8 UChessSessionResultHelper::GetMaxPlayers(const FChessSessionSearchResult& search) {
    return search.Result.Session.SessionSettings.NumPublicConnections;
}
