// Copyright 2018 Emre Simsili

#pragma once

#include "Engine/GameInstance.h"
#include "State.h"
#include "DestroySessionCallbackProxy.h"
#include "Online/ChessSessionResultHelper.h"
#include "ChessGameInstance.generated.h"

UCLASS()
class CHESS_API UChessGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Gameplay")
    TEnumAsByte<EState::Type> State;

    UChessGameInstance();

    UFUNCTION(BlueprintCallable, Category = "Online|Session")
    bool HostSession(FString name, bool is_lan = true);
    UFUNCTION(BlueprintCallable, Category = "Online|Session")
    bool FindSessions(bool is_lan = true);
    UFUNCTION(BlueprintCallable, Category = "Online|Session")
    bool JoinToSession(const FChessSessionSearchResult& session);
    UFUNCTION(BlueprintCallable, Category = "Online|Session")
    bool DestroySession();

    UFUNCTION(BlueprintImplementableEvent)
    void OnSessionStart(bool is_successful);
    UFUNCTION(BlueprintImplementableEvent)
    void OnSessionsFound(UPARAM(ref) TArray<FChessSessionSearchResult>& sessions);
    UFUNCTION(BlueprintImplementableEvent)
    void OnSessionJoined(bool is_successful);
    UFUNCTION(BlueprintImplementableEvent)
    void OnSessionDestroyed(bool is_successful);

private:
    TSharedPtr<class FOnlineSessionSettings> SessionSettings;
    TSharedPtr<class FOnlineSessionSearch> SessionSearch;

    FDelegateHandle OnCreateSessionCompleteDelegateHandle;
    FOnCreateSessionCompleteDelegate OnCreateSessionCompleteDelegate;
    FDelegateHandle OnStartSessionCompleteDelegateHandle;
    FOnStartSessionCompleteDelegate OnStartSessionCompleteDelegate;
    FDelegateHandle OnFindSessionsCompleteDelegateHandle;
    FOnFindSessionsCompleteDelegate OnFindSessionsCompleteDelegate;
    FDelegateHandle OnJoinSessionCompleteDelegateHandle;
    FOnJoinSessionCompleteDelegate OnJoinSessionCompleteDelegate;
    FDelegateHandle OnDestroySessionCompleteDelegateHandle;
    FOnDestroySessionCompleteDelegate OnDestroySessionCompleteDelegate;

    bool HostSession(TSharedPtr<const FUniqueNetId> user_id, FString session_name, bool is_lan);
    bool FindSessions(TSharedPtr<const FUniqueNetId> user_id, bool is_lan);
    bool JoinToSession(TSharedPtr<const FUniqueNetId> user_id,
                       const FOnlineSessionSearchResult& search_result);

    void OnCreateSessionComplete(FName session_name, bool successful);
    void OnStartSessionComplete(FName session_name, bool successful);
    void OnFindSessionsComplete(bool successful);
    void OnJoinSessionComplete(FName session_name, EOnJoinSessionCompleteResult::Type result);
    void OnDestroySessionComplete(FName session_name, bool successful);
};
