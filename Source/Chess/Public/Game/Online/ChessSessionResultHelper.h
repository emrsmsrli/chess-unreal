// Copyright 2018 Emre Simsirli

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Online.h"
#include "ChessSessionResultHelper.generated.h"

#define KEY_LOBBY_NAME FName("Lobby")

USTRUCT(BlueprintType)
struct FChessSessionSearchResult {
	GENERATED_BODY()

    FOnlineSessionSearchResult Result;
};

UCLASS(MinimalApi)
class UChessSessionResultHelper : public UBlueprintFunctionLibrary {
    GENERATED_BODY()

    UFUNCTION(BlueprintPure, Category = "Online|Session")
    static FString GetName(const FChessSessionSearchResult& search);

    UFUNCTION(BlueprintPure, Category = "Online|Session")
    static int32 GetPing(const FChessSessionSearchResult& search);

    UFUNCTION(BlueprintPure, Category = "Online|Session")
    static uint8 GetCurrentPlayers(const FChessSessionSearchResult& search);

    UFUNCTION(BlueprintPure, Category = "Online|Session")
    static uint8 GetMaxPlayers(const FChessSessionSearchResult& search);
};
