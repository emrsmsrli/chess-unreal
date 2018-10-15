// Copyright 2018 Emre Simsirli

#include "ChessGameInstance.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/LocalPlayer.h"

UChessGameInstance::UChessGameInstance()
{
    State = EState::START;
    OnCreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(
        this, &UChessGameInstance::OnCreateSessionComplete);
    OnStartSessionCompleteDelegate = FOnStartSessionCompleteDelegate::CreateUObject(
        this, &UChessGameInstance::OnStartSessionComplete);
    OnFindSessionsCompleteDelegate = FOnFindSessionsCompleteDelegate::CreateUObject(
        this, &UChessGameInstance::OnFindSessionsComplete);
    OnJoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(
        this, &UChessGameInstance::OnJoinSessionComplete);
    OnDestroySessionCompleteDelegate = FOnDestroySessionCompleteDelegate::CreateUObject(
        this, &UChessGameInstance::OnDestroySessionComplete);
}

bool UChessGameInstance::HostSession(const FString name, const bool is_lan)
{
    auto* player = GetFirstGamePlayer();
    return HostSession(player->GetPreferredUniqueNetId().GetUniqueNetId(), name, is_lan);
}

bool UChessGameInstance::FindSessions(const bool is_lan)
{
    auto* player = GetFirstGamePlayer();
    return FindSessions(player->GetPreferredUniqueNetId().GetUniqueNetId(), is_lan);
}

bool UChessGameInstance::JoinToSession(const FChessSessionSearchResult& search)
{
    auto* player = GetFirstGamePlayer();
    return JoinToSession(player->GetPreferredUniqueNetId().GetUniqueNetId(), search.Result);
}

bool UChessGameInstance::DestroySession()
{
    const auto* online_sub = IOnlineSubsystem::Get();
    if(!online_sub) {
        UE_LOG(LogTemp, Log, TEXT("UChessGameInstance::DestroySession IOnlineSubsystem::Get() nullptr"));
        return false;
    }

    const auto sessions = online_sub->GetSessionInterface();
    if(!sessions.IsValid()) {
        UE_LOG(LogTemp, Log, TEXT("UChessGameInstance::DestroySession GetSessionInterface() not valid"));
        return false;
    }

    sessions->AddOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegate);
    return sessions->DestroySession(GameSessionName);
}

bool UChessGameInstance::HostSession(const TSharedPtr<const FUniqueNetId> user_id, const FString session_name,
                                     const bool is_lan)
{
    const auto* online_sub = IOnlineSubsystem::Get();
    if(!online_sub) {
        UE_LOG(LogTemp, Log, TEXT("UChessGameInstance::HostSession IOnlineSubsystem::Get() nullptr"));
        return false;
    }

    auto sessions = online_sub->GetSessionInterface();
    if(!sessions.IsValid() || !user_id.IsValid()) {
        UE_LOG(LogTemp, Log, TEXT(" UChessGameInstance::HostSession GetSessionInterface() or user_id not valid"));
        return false;
    }

    SessionSettings = MakeShareable(new FOnlineSessionSettings());
    SessionSettings->bIsLANMatch = is_lan;
    SessionSettings->bUsesPresence = true;
    SessionSettings->NumPublicConnections = 2;
    SessionSettings->NumPrivateConnections = 0;
    SessionSettings->bAllowJoinInProgress = true;
    SessionSettings->bShouldAdvertise = true;
    SessionSettings->bAllowJoinViaPresence = true;
    SessionSettings->bAllowJoinViaPresenceFriendsOnly = false;

    FOnlineSessionSetting lobby_name_setting;
    lobby_name_setting.Data = session_name;
    lobby_name_setting.AdvertisementType = EOnlineDataAdvertisementType::ViaOnlineService;
    SessionSettings->Settings.Add(KEY_LOBBY_NAME, lobby_name_setting);

    OnCreateSessionCompleteDelegateHandle = sessions->AddOnCreateSessionCompleteDelegate_Handle(
        OnCreateSessionCompleteDelegate);

    return sessions->CreateSession(*user_id, GameSessionName, *SessionSettings);
}

bool UChessGameInstance::FindSessions(TSharedPtr<const FUniqueNetId> user_id, const bool is_lan)
{
    const auto* online_sub = IOnlineSubsystem::Get();
    if(!online_sub) {
        UE_LOG(LogTemp, Log, TEXT(" UChessGameInstance::FindSessions IOnlineSubsystem::Get() nullptr"));
        return false;
    }

    const auto sessions = online_sub->GetSessionInterface();
    if(!sessions.IsValid() || !user_id.IsValid()) {
        UE_LOG(LogTemp, Log, TEXT(" UChessGameInstance::FindSessions GetSessionInterface() or user_id not valid"));
        return false;
    }

    SessionSearch = MakeShareable(new FOnlineSessionSearch());
    SessionSearch->bIsLanQuery = is_lan;
    SessionSearch->MaxSearchResults = 20;
    SessionSearch->PingBucketSize = 50;
    SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

    OnFindSessionsCompleteDelegateHandle = sessions->AddOnFindSessionsCompleteDelegate_Handle(
        OnFindSessionsCompleteDelegate);

    return sessions->FindSessions(*user_id, SessionSearch.ToSharedRef());
}

bool UChessGameInstance::JoinToSession(TSharedPtr<const FUniqueNetId> user_id,
                                       const FOnlineSessionSearchResult& search_result)
{
    const auto* online_sub = IOnlineSubsystem::Get();
    if(!online_sub) {
        UE_LOG(LogTemp, Log, TEXT("UChessGameInstance::JoinToSession IOnlineSubsystem::Get() nullptr"));
        return false;
    }

    const auto sessions = online_sub->GetSessionInterface();
    if(!sessions.IsValid() || !user_id.IsValid()) {
        UE_LOG(LogTemp, Log, TEXT("UChessGameInstance::JoinToSession GetSessionInterface() or user_id not valid"));
        return false;
    }

    OnJoinSessionCompleteDelegateHandle = sessions->AddOnJoinSessionCompleteDelegate_Handle(
        OnJoinSessionCompleteDelegate);

    return sessions->JoinSession(*user_id, GameSessionName, search_result);
}

void UChessGameInstance::OnCreateSessionComplete(const FName session_name, const bool successful)
{
    UE_LOG(LogTemp, Log, TEXT("UChessGameInstance::OnCreateSessionComplete, %s, success: %s"),
        *session_name.ToString(), successful ? TEXT("true") : TEXT("false"));

    const auto* online_sub = IOnlineSubsystem::Get();
    check(online_sub);
    auto sessions = online_sub->GetSessionInterface();
    check(sessions.IsValid());

    sessions->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegateHandle);
    if(!successful) {
        OnSessionStart(false);
        return;
    }

    OnStartSessionCompleteDelegateHandle = sessions->AddOnStartSessionCompleteDelegate_Handle(
        OnStartSessionCompleteDelegate);

    sessions->StartSession(session_name);
}

void UChessGameInstance::OnStartSessionComplete(const FName session_name, const bool successful)
{
    UE_LOG(LogTemp, Log, TEXT("UChessGameInstance::OnStartSessionComplete, %s, success: %s"),
        *session_name.ToString(), successful ? TEXT("true") : TEXT("false"));

    const auto* online_sub = IOnlineSubsystem::Get();
    check(online_sub);

    auto sessions = online_sub->GetSessionInterface();
    check(sessions);
    sessions->ClearOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegateHandle);

    OnSessionStart(successful);
}

void UChessGameInstance::OnFindSessionsComplete(const bool successful)
{
    UE_LOG(LogTemp, Log, TEXT("UChessGameInstance::OnFindSessionsComplete, success: %s"),
        successful ? TEXT("true") : TEXT("false"));

    const auto* online_sub = IOnlineSubsystem::Get();
    check(online_sub);
    const auto sessions = online_sub->GetSessionInterface();
    check(sessions.IsValid());

    sessions->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegateHandle);

    UE_LOG(LogTemp, Log, TEXT("UChessGameInstance::OnFindSessionsComplete, session count: %d"),
        SessionSearch->SearchResults.Num());

    TArray<FChessSessionSearchResult> results;
    if(SessionSearch->SearchResults.Num() > 0) {
        for(auto i = 0; i < SessionSearch->SearchResults.Num(); i++) {
            auto result = FChessSessionSearchResult();
            result.Result = SessionSearch->SearchResults[i];
            results.Add(result);
        }
    }

    OnSessionsFound(results);
}

void UChessGameInstance::OnJoinSessionComplete(const FName session_name,
                                               const EOnJoinSessionCompleteResult::Type result)
{
    UE_LOG(LogTemp, Log, TEXT("UChessGameInstance::OnJoinSessionComplete, %s, result: %d"),
        *session_name.ToString(),
        static_cast<int32>(result));

    const auto* online_sub = IOnlineSubsystem::Get();
    check(online_sub);
    const auto sessions = online_sub->GetSessionInterface();
    check(sessions.IsValid())

    sessions->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegateHandle);

    auto* controller = GetFirstLocalPlayerController();

    FString connect_info;
    if(!controller || !sessions->GetResolvedConnectString(session_name, connect_info))
        return;

    controller->ClientTravel(connect_info, ETravelType::TRAVEL_Absolute);
    OnSessionJoined(result == EOnJoinSessionCompleteResult::Success ? true : false);
}

void UChessGameInstance::OnDestroySessionComplete(const FName session_name, const bool successful)
{
    UE_LOG(LogTemp, Log, TEXT("UChessGameInstance::OnDestroySessionComplete, %s, success: %s"),
        *session_name.ToString(),
        successful ? TEXT("true") : TEXT("false"));

    const auto* online_sub = IOnlineSubsystem::Get();
    check(online_sub);
    const auto sessions = online_sub->GetSessionInterface();
    check(sessions.IsValid());

    sessions->ClearOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegateHandle);
    OnSessionDestroyed(successful);
}
