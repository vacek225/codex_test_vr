#include "Core/KemRoomSubsystem.h"

#include "Core/KemSettings.h"
#include "Actors/KemClassroomStateActor.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "Engine/LocalPlayer.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"

const FName UKemRoomSubsystem::SessionName(TEXT("KemSUinVRSession"));
const FName UKemRoomSubsystem::RoomCodeKey(TEXT("KEM_ROOM_CODE"));
const FName UKemRoomSubsystem::AuditoriumKey(TEXT("KEM_AUDITORIUM"));

void UKemRoomSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    PostWorldInitHandle = FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &UKemRoomSubsystem::HandlePostWorldInitialization);
}

void UKemRoomSubsystem::Deinitialize()
{
    if (PostWorldInitHandle.IsValid())
    {
        FWorldDelegates::OnPostWorldInitialization.Remove(PostWorldInitHandle);
        PostWorldInitHandle.Reset();
    }

    ClearSessionDelegates();
    Super::Deinitialize();
}

TArray<FKemAuditoriumDefinition> UKemRoomSubsystem::GetAuditoriums() const
{
    return GetDefault<UKemSUinVRSettings>()->Auditoriums;
}

void UKemRoomSubsystem::HostAuditorium(FName AuditoriumId)
{
    if (bOperationInFlight)
    {
        OnHostOperationFinished.Broadcast(false, TEXT("Another room operation is already in progress."));
        return;
    }

    const UKemSUinVRSettings* Settings = GetDefault<UKemSUinVRSettings>();
    const FKemAuditoriumDefinition* Auditorium = Settings->FindAuditoriumById(AuditoriumId);
    if (!Auditorium)
    {
        OnHostOperationFinished.Broadcast(false, TEXT("Auditorium definition was not found."));
        return;
    }

    StartHostingAuditorium(*Auditorium);
}

void UKemRoomSubsystem::JoinRoomByCode(const FString& RoomCode)
{
    if (bOperationInFlight)
    {
        OnJoinOperationFinished.Broadcast(false, TEXT("Another room operation is already in progress."));
        return;
    }

    IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
    if (!OnlineSubsystem)
    {
        OnJoinOperationFinished.Broadcast(false, TEXT("No online subsystem is active. Check OnlineSubsystemNull settings."));
        return;
    }

    IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
    if (!SessionInterface.IsValid())
    {
        OnJoinOperationFinished.Broadcast(false, TEXT("Session interface is not available."));
        return;
    }

    PendingJoinCode = SanitizeRoomCode(RoomCode);
    ActiveSessionSearch = MakeShared<FOnlineSessionSearch>();
    ActiveSessionSearch->bIsLanQuery = true;
    ActiveSessionSearch->MaxSearchResults = 128;
    ActiveSessionSearch->PingBucketSize = 50;

    bOperationInFlight = true;
    FindSessionsHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(
        FOnFindSessionsCompleteDelegate::CreateUObject(this, &UKemRoomSubsystem::HandleFindSessionsComplete));

    if (!SessionInterface->FindSessions(0, ActiveSessionSearch.ToSharedRef()))
    {
        ClearSessionDelegates();
        bOperationInFlight = false;
        OnJoinOperationFinished.Broadcast(false, TEXT("Failed to start LAN session search."));
    }
}

void UKemRoomSubsystem::LeaveCurrentRoom()
{
    IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
    IOnlineSessionPtr SessionInterface = OnlineSubsystem ? OnlineSubsystem->GetSessionInterface() : nullptr;

    if (SessionInterface.IsValid())
    {
        DestroySessionHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
            FOnDestroySessionCompleteDelegate::CreateUObject(this, &UKemRoomSubsystem::HandleDestroySessionComplete));
        SessionInterface->DestroySession(SessionName);
    }

    CachedClassroomState = nullptr;
    CurrentRoomInfo = FKemRoomInfo();
    PendingHostedRoomInfo = FKemRoomInfo();
    bAwaitingTravelBootstrap = false;
    BroadcastRoomInfo();

    const UKemSUinVRSettings* Settings = GetDefault<UKemSUinVRSettings>();
    const FString FallbackMap = Settings->Auditoriums.IsEmpty() ? TEXT("/Game/VRTemplate/VRTemplateMap") : Settings->Auditoriums[0].MapPath;
    if (UWorld* World = GetWorld())
    {
        UGameplayStatics::OpenLevel(World, FName(*FallbackMap));
    }
}

AKemClassroomStateActor* UKemRoomSubsystem::GetCurrentClassroomState() const
{
    return CachedClassroomState.Get();
}

void UKemRoomSubsystem::NotifyClassroomStateReady(AKemClassroomStateActor* InStateActor)
{
    CachedClassroomState = InStateActor;
}

void UKemRoomSubsystem::StartHostingAuditorium(const FKemAuditoriumDefinition& Auditorium)
{
    IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
    if (!OnlineSubsystem)
    {
        OnHostOperationFinished.Broadcast(false, TEXT("No online subsystem is active. Check OnlineSubsystemNull settings."));
        return;
    }

    IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
    if (!SessionInterface.IsValid())
    {
        OnHostOperationFinished.Broadcast(false, TEXT("Session interface is not available."));
        return;
    }

    FOnlineSessionSettings SessionSettings;
    SessionSettings.bIsLANMatch = true;
    SessionSettings.bAllowJoinInProgress = true;
    SessionSettings.bAllowJoinViaPresence = true;
    SessionSettings.bShouldAdvertise = true;
    SessionSettings.bUsesPresence = true;
    SessionSettings.NumPublicConnections = GetDefault<UKemSUinVRSettings>()->DefaultPublicConnections;
    SessionSettings.Set(RoomCodeKey, GenerateRoomCode(GetDefault<UKemSUinVRSettings>()->RoomCodeLength), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
    SessionSettings.Set(AuditoriumKey, Auditorium.AuditoriumId.ToString(), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
    SessionSettings.Set(FName(TEXT("MAPNAME")), Auditorium.MapPath, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

    PendingHostedRoomInfo = FKemRoomInfo();
    SessionSettings.Get(RoomCodeKey, PendingHostedRoomInfo.RoomCode);
    PendingHostedRoomInfo.AuditoriumId = Auditorium.AuditoriumId;
    PendingHostedRoomInfo.MapPath = Auditorium.MapPath;
    PendingHostedRoomInfo.bIsHost = true;
    PendingHostedRoomInfo.bIsInRoom = true;

    bOperationInFlight = true;
    CreateSessionHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
        FOnCreateSessionCompleteDelegate::CreateUObject(this, &UKemRoomSubsystem::HandleCreateSessionComplete));

    if (!SessionInterface->CreateSession(0, SessionName, SessionSettings))
    {
        ClearSessionDelegates();
        bOperationInFlight = false;
        PendingHostedRoomInfo = FKemRoomInfo();
        OnHostOperationFinished.Broadcast(false, TEXT("Failed to start LAN room creation."));
    }
}

FString UKemRoomSubsystem::GenerateRoomCode(int32 CodeLength) const
{
    static const TCHAR Alphabet[] = TEXT("ABCDEFGHJKLMNPQRSTUVWXYZ23456789");
    FString Generated;
    Generated.Reserve(CodeLength);

    for (int32 Index = 0; Index < CodeLength; ++Index)
    {
        const int32 AlphabetIndex = FMath::RandRange(0, UE_ARRAY_COUNT(Alphabet) - 2);
        Generated.AppendChar(Alphabet[AlphabetIndex]);
    }

    return Generated;
}

FString UKemRoomSubsystem::SanitizeRoomCode(const FString& RoomCode) const
{
    FString Sanitized = RoomCode.ToUpper();
    Sanitized.RemoveSpacesInline();
    Sanitized.ReplaceInline(TEXT("-"), TEXT(""));
    return Sanitized;
}

void UKemRoomSubsystem::BroadcastRoomInfo()
{
    OnRoomInfoChanged.Broadcast(CurrentRoomInfo);
}

void UKemRoomSubsystem::BootstrapRoomAfterTravel(UWorld* World)
{
    if (!World || !World->IsGameWorld())
    {
        return;
    }

    EnsureVoiceEnabledForLocalPlayer(World);

    if (!bAwaitingTravelBootstrap || !CurrentRoomInfo.bIsHost)
    {
        return;
    }

    if (World->GetNetMode() == NM_Client)
    {
        return;
    }

    AKemClassroomStateActor* ExistingState = nullptr;
    for (TActorIterator<AKemClassroomStateActor> It(World); It; ++It)
    {
        ExistingState = *It;
        break;
    }

    if (!ExistingState)
    {
        ExistingState = World->SpawnActor<AKemClassroomStateActor>();
    }

    if (ExistingState)
    {
        ExistingState->InitializeHostedRoom(CurrentRoomInfo, GetDefault<UKemSUinVRSettings>()->DefaultLectureUrl);
        CachedClassroomState = ExistingState;
        bAwaitingTravelBootstrap = false;
    }
}

void UKemRoomSubsystem::EnsureVoiceEnabledForLocalPlayer(UWorld* World) const
{
    if (!World)
    {
        return;
    }

    if (APlayerController* PlayerController = World->GetFirstPlayerController())
    {
        PlayerController->ConsoleCommand(TEXT("ToggleSpeaking 1"), true);
    }
}

void UKemRoomSubsystem::HandlePostWorldInitialization(UWorld* World, const UWorld::InitializationValues IVS)
{
    if (!World || !World->IsGameWorld())
    {
        return;
    }

    World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &UKemRoomSubsystem::BootstrapRoomAfterTravel, World));
}

void UKemRoomSubsystem::HandleCreateSessionComplete(FName InSessionName, bool bWasSuccessful)
{
    bOperationInFlight = false;

    IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
    if (IOnlineSessionPtr SessionInterface = OnlineSubsystem ? OnlineSubsystem->GetSessionInterface() : nullptr)
    {
        SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionHandle);
    }
    CreateSessionHandle.Reset();

    if (!bWasSuccessful)
    {
        PendingHostedRoomInfo = FKemRoomInfo();
        OnHostOperationFinished.Broadcast(false, TEXT("LAN room creation failed."));
        return;
    }

    CurrentRoomInfo = PendingHostedRoomInfo;
    PendingHostedRoomInfo = FKemRoomInfo();
    bAwaitingTravelBootstrap = true;
    BroadcastRoomInfo();
    OnHostOperationFinished.Broadcast(true, FString::Printf(TEXT("Room created. Code: %s"), *CurrentRoomInfo.RoomCode));

    if (UWorld* World = GetWorld())
    {
        UGameplayStatics::OpenLevel(World, FName(*CurrentRoomInfo.MapPath), true, TEXT("listen"));
    }
}

void UKemRoomSubsystem::HandleFindSessionsComplete(bool bWasSuccessful)
{
    bOperationInFlight = false;

    IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
    IOnlineSessionPtr SessionInterface = OnlineSubsystem ? OnlineSubsystem->GetSessionInterface() : nullptr;
    if (SessionInterface.IsValid())
    {
        SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsHandle);
    }
    FindSessionsHandle.Reset();

    if (!bWasSuccessful || !ActiveSessionSearch.IsValid())
    {
        OnJoinOperationFinished.Broadcast(false, TEXT("LAN session search failed."));
        return;
    }

    for (const FOnlineSessionSearchResult& Result : ActiveSessionSearch->SearchResults)
    {
        FString FoundCode;
        if (Result.Session.SessionSettings.Get(RoomCodeKey, FoundCode) && SanitizeRoomCode(FoundCode) == PendingJoinCode)
        {
            JoinSessionHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
                FOnJoinSessionCompleteDelegate::CreateUObject(this, &UKemRoomSubsystem::HandleJoinSessionComplete));

            if (!SessionInterface->JoinSession(0, SessionName, Result))
            {
                SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionHandle);
                JoinSessionHandle.Reset();
                OnJoinOperationFinished.Broadcast(false, TEXT("JoinSession failed to start."));
                return;
            }

            return;
        }
    }

    OnJoinOperationFinished.Broadcast(false, TEXT("Room with this code was not found on LAN."));
}

void UKemRoomSubsystem::HandleJoinSessionComplete(FName InSessionName, EOnJoinSessionCompleteResult::Type Result)
{
    IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
    IOnlineSessionPtr SessionInterface = OnlineSubsystem ? OnlineSubsystem->GetSessionInterface() : nullptr;
    if (SessionInterface.IsValid())
    {
        SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionHandle);
    }
    JoinSessionHandle.Reset();

    if (!SessionInterface.IsValid() || Result != EOnJoinSessionCompleteResult::Success)
    {
        OnJoinOperationFinished.Broadcast(false, TEXT("Joining the room failed."));
        return;
    }

    FString ConnectString;
    if (!SessionInterface->GetResolvedConnectString(SessionName, ConnectString))
    {
        OnJoinOperationFinished.Broadcast(false, TEXT("Could not resolve the host address."));
        return;
    }

    CurrentRoomInfo.RoomCode = PendingJoinCode;
    CurrentRoomInfo.bIsHost = false;
    CurrentRoomInfo.bIsInRoom = true;
    BroadcastRoomInfo();
    OnJoinOperationFinished.Broadcast(true, FString::Printf(TEXT("Joining room %s"), *PendingJoinCode));

    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PlayerController = World->GetFirstPlayerController())
        {
            PlayerController->ClientTravel(ConnectString, ETravelType::TRAVEL_Absolute);
        }
    }
}

void UKemRoomSubsystem::HandleDestroySessionComplete(FName InSessionName, bool bWasSuccessful)
{
    IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
    IOnlineSessionPtr SessionInterface = OnlineSubsystem ? OnlineSubsystem->GetSessionInterface() : nullptr;
    if (SessionInterface.IsValid())
    {
        SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionHandle);
    }
    DestroySessionHandle.Reset();
}

void UKemRoomSubsystem::ClearSessionDelegates()
{
    IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
    IOnlineSessionPtr SessionInterface = OnlineSubsystem ? OnlineSubsystem->GetSessionInterface() : nullptr;
    if (!SessionInterface.IsValid())
    {
        return;
    }

    if (CreateSessionHandle.IsValid())
    {
        SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionHandle);
        CreateSessionHandle.Reset();
    }

    if (FindSessionsHandle.IsValid())
    {
        SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsHandle);
        FindSessionsHandle.Reset();
    }

    if (JoinSessionHandle.IsValid())
    {
        SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionHandle);
        JoinSessionHandle.Reset();
    }

    if (DestroySessionHandle.IsValid())
    {
        SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionHandle);
        DestroySessionHandle.Reset();
    }
}
