#include "Actors/KemWorldActors.h"

#include "Actors/KemClassroomStateActor.h"
#include "Actors/KemDesktopCharacter.h"
#include "Actors/KemPlayerControlProxy.h"
#include "Components/WidgetComponent.h"
#include "Core/KemRoomSubsystem.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/Pawn.h"
#include "IXRTrackingSystem.h"
#include "UI/KemWidgets.h"

AKemWidgetPanelActorBase::AKemWidgetPanelActorBase()
{
    PrimaryActorTick.bCanEverTick = true;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;

    WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetComponent"));
    WidgetComponent->SetupAttachment(SceneRoot);
    WidgetComponent->SetWidgetSpace(EWidgetSpace::World);
    WidgetComponent->SetDrawAtDesiredSize(false);
    WidgetComponent->SetTwoSided(true);
    WidgetComponent->SetCollisionProfileName(TEXT("UI"));
    WidgetComponent->SetPivot(FVector2D(0.5f, 0.5f));
}

void AKemWidgetPanelActorBase::BeginPlay()
{
    Super::BeginPlay();
    WidgetComponent->InitWidget();
}

void AKemWidgetPanelActorBase::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (bFaceLocalPlayer)
    {
        FaceFirstPlayer();
    }
}

void AKemWidgetPanelActorBase::FaceFirstPlayer()
{
    if (APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr)
    {
        FVector ViewLocation = FVector::ZeroVector;
        FRotator ViewRotation = FRotator::ZeroRotator;
        PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);

        const FVector ToViewer = ViewLocation - GetActorLocation();
        SetActorRotation(ToViewer.Rotation());
    }
}

AKemHubActor::AKemHubActor()
{
    bFaceLocalPlayer = true;
    WidgetComponent->SetWidgetClass(UKemHubMenuWidget::StaticClass());
    WidgetComponent->SetDrawSize(FVector2D(1100.0f, 1200.0f));
    SetActorScale3D(FVector(0.08f));
}

void AKemHubActor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
    APawn* PlayerPawn = PlayerController ? PlayerController->GetPawn() : nullptr;
    UKemRoomSubsystem* RoomSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UKemRoomSubsystem>() : nullptr;
    const bool bDesktopMenuOwnsPresentation = PlayerPawn && PlayerPawn->IsA<AKemDesktopCharacter>() && RoomSubsystem && !RoomSubsystem->GetCurrentRoomInfo().bIsInRoom;
    SetActorHiddenInGame(bDesktopMenuOwnsPresentation);
}

AKemRoomCodeDisplayActor::AKemRoomCodeDisplayActor()
{
    bFaceLocalPlayer = true;
    WidgetComponent->SetWidgetClass(UKemRoomCodeWidget::StaticClass());
    WidgetComponent->SetDrawSize(FVector2D(700.0f, 280.0f));
    SetActorScale3D(FVector(0.08f));
}

void AKemRoomCodeDisplayActor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (UKemRoomSubsystem* RoomSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UKemRoomSubsystem>() : nullptr)
    {
        SetActorHiddenInGame(!RoomSubsystem->IsHost());
    }
}

AKemTeacherControlPanelActor::AKemTeacherControlPanelActor()
{
    bFaceLocalPlayer = true;
    WidgetComponent->SetWidgetClass(UKemTeacherControlWidget::StaticClass());
    WidgetComponent->SetDrawSize(FVector2D(900.0f, 1200.0f));
    SetActorScale3D(FVector(0.07f));
}

void AKemTeacherControlPanelActor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (UKemRoomSubsystem* RoomSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UKemRoomSubsystem>() : nullptr)
    {
        SetActorHiddenInGame(!RoomSubsystem->IsHost());
    }
}

AKemBrowserDeskActor::AKemBrowserDeskActor()
{
    bFaceLocalPlayer = true;
    WidgetComponent->SetWidgetClass(UKemBrowserDeskWidget::StaticClass());
    WidgetComponent->SetDrawSize(FVector2D(1200.0f, 1100.0f));
    SetActorScale3D(FVector(0.06f));
}

void AKemBrowserDeskActor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (UKemRoomSubsystem* RoomSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UKemRoomSubsystem>() : nullptr)
    {
        SetActorHiddenInGame(!RoomSubsystem->IsHost());
    }
}

AKemSharedLectureScreenActor::AKemSharedLectureScreenActor()
{
    bFaceLocalPlayer = true;
    WidgetComponent->SetWidgetClass(UKemLectureScreenWidget::StaticClass());
    WidgetComponent->SetDrawSize(FVector2D(1600.0f, 1100.0f));
    SetActorScale3D(FVector(0.09f));
}

void AKemSharedLectureScreenActor::BeginPlay()
{
    Super::BeginPlay();

    if (UKemLectureScreenWidget* ScreenWidget = Cast<UKemLectureScreenWidget>(WidgetComponent->GetUserWidgetObject()))
    {
        ScreenWidget->SetAllowLocalCloneButton(true);
    }
}

AKemLocalLectureScreenActor::AKemLocalLectureScreenActor()
{
    bReplicates = false;
    bFaceLocalPlayer = true;
    WidgetComponent->SetWidgetClass(UKemLectureScreenWidget::StaticClass());
    WidgetComponent->SetDrawSize(FVector2D(1200.0f, 850.0f));
    SetActorScale3D(FVector(0.07f));
}

void AKemLocalLectureScreenActor::BeginPlay()
{
    Super::BeginPlay();

    if (UKemLectureScreenWidget* ScreenWidget = Cast<UKemLectureScreenWidget>(WidgetComponent->GetUserWidgetObject()))
    {
        ScreenWidget->SetAllowLocalCloneButton(false);
    }
}

AKemParticipantModeratorActor::AKemParticipantModeratorActor()
{
    bReplicates = false;
    bFaceLocalPlayer = true;
    WidgetComponent->SetWidgetClass(UKemParticipantModeratorWidget::StaticClass());
    WidgetComponent->SetDrawSize(FVector2D(700.0f, 620.0f));
    SetActorScale3D(FVector(0.06f));
}

void AKemParticipantModeratorActor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!ObservedPawn.IsValid())
    {
        Destroy();
        return;
    }

    SetActorLocation(ObservedPawn->GetActorLocation() + FVector(0.0f, 0.0f, 220.0f));
}

void AKemParticipantModeratorActor::BindToParticipant(APawn* TargetPawn, const FKemParticipantInfo& ParticipantInfo)
{
    ObservedPawn = TargetPawn;
    CachedParticipantInfo = ParticipantInfo;

    if (UKemParticipantModeratorWidget* ModeratorWidget = Cast<UKemParticipantModeratorWidget>(WidgetComponent->GetUserWidgetObject()))
    {
        ModeratorWidget->SetParticipantInfo(CachedParticipantInfo);
    }
}

bool AKemParticipantModeratorActor::IsBoundToPawn(const APawn* Pawn) const
{
    return ObservedPawn.Get() == Pawn;
}

AKemHostModeratorManagerActor::AKemHostModeratorManagerActor()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AKemHostModeratorManagerActor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    RefreshAccumulator += DeltaSeconds;
    if (RefreshAccumulator < 0.5f)
    {
        return;
    }
    RefreshAccumulator = 0.0f;

    UKemRoomSubsystem* RoomSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UKemRoomSubsystem>() : nullptr;
    AKemClassroomStateActor* RoomState = RoomSubsystem ? RoomSubsystem->GetCurrentClassroomState() : nullptr;
    if (!RoomSubsystem || !RoomSubsystem->IsHost() || !RoomState)
    {
        for (TPair<TWeakObjectPtr<APawn>, TObjectPtr<AKemParticipantModeratorActor>>& Entry : ModeratorPanels)
        {
            if (Entry.Value)
            {
                Entry.Value->Destroy();
            }
        }
        ModeratorPanels.Reset();
        return;
    }

    APawn* LocalPawn = GetWorld() && GetWorld()->GetFirstPlayerController() ? GetWorld()->GetFirstPlayerController()->GetPawn() : nullptr;
    TSet<TWeakObjectPtr<APawn>> SeenPawns;

    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PlayerController = It->Get();
        APawn* Pawn = PlayerController ? PlayerController->GetPawn() : nullptr;
        if (!Pawn || Pawn == LocalPawn || !PlayerController->PlayerState)
        {
            continue;
        }

        const FString ParticipantId = PlayerController->PlayerState->GetUniqueId().IsValid()
            ? PlayerController->PlayerState->GetUniqueId()->ToString()
            : PlayerController->PlayerState->GetPlayerName();

        const FKemParticipantInfo* Participant = RoomState->GetParticipants().FindByPredicate([&ParticipantId](const FKemParticipantInfo& Entry)
        {
            return Entry.ParticipantId == ParticipantId;
        });

        if (!Participant)
        {
            continue;
        }

        SeenPawns.Add(Pawn);

        TObjectPtr<AKemParticipantModeratorActor>* ExistingPanel = ModeratorPanels.Find(Pawn);
        if (!ExistingPanel || !*ExistingPanel)
        {
            FActorSpawnParameters SpawnParameters;
            SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            AKemParticipantModeratorActor* NewPanel = GetWorld()->SpawnActor<AKemParticipantModeratorActor>(Pawn->GetActorLocation() + FVector(0.0f, 0.0f, 220.0f), FRotator::ZeroRotator, SpawnParameters);
            if (NewPanel)
            {
                NewPanel->BindToParticipant(Pawn, *Participant);
                ModeratorPanels.Add(Pawn, NewPanel);
            }
        }
        else
        {
            (*ExistingPanel)->BindToParticipant(Pawn, *Participant);
        }
    }

    for (auto It = ModeratorPanels.CreateIterator(); It; ++It)
    {
        if (!It.Key().IsValid() || !SeenPawns.Contains(It.Key()))
        {
            if (It.Value())
            {
                It.Value()->Destroy();
            }
            It.RemoveCurrent();
        }
    }
}

AKemClassroomBootstrapActor::AKemClassroomBootstrapActor()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AKemClassroomBootstrapActor::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        EnsurePlayerControlProxies();
    }
}

void AKemClassroomBootstrapActor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (HasAuthority())
    {
        ProxyRefreshAccumulator += DeltaSeconds;
        if (ProxyRefreshAccumulator >= 0.5f)
        {
            ProxyRefreshAccumulator = 0.0f;
            EnsurePlayerControlProxies();
        }
    }

    EnsureLocalPlayerControlMode();
    EnsureLocalLayoutInitialized();
}

void AKemClassroomBootstrapActor::EnsureLocalLayoutInitialized()
{
    APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
    APawn* PlayerPawn = PlayerController ? PlayerController->GetPawn() : nullptr;
    if (!PlayerController || !PlayerPawn)
    {
        return;
    }

    const bool bShouldRefreshLayout = !bLocalLayoutInitialized || LastLayoutPawn.Get() != PlayerPawn;
    if (!bShouldRefreshLayout)
    {
        return;
    }

    const FVector LayoutOrigin = PlayerPawn->GetActorLocation();
    const FRotator LayoutRotation(0.0f, PlayerController->GetControlRotation().Yaw, 0.0f);

    SpawnIfMissing<AKemHubActor>(BuildLayoutTransform(LayoutOrigin, LayoutRotation, HubOffset));
    SpawnIfMissing<AKemRoomCodeDisplayActor>(BuildLayoutTransform(LayoutOrigin, LayoutRotation, RoomCodeOffset));
    SpawnIfMissing<AKemTeacherControlPanelActor>(BuildLayoutTransform(LayoutOrigin, LayoutRotation, TeacherPanelOffset));
    SpawnIfMissing<AKemBrowserDeskActor>(BuildLayoutTransform(LayoutOrigin, LayoutRotation, BrowserDeskOffset));
    SpawnIfMissing<AKemSharedLectureScreenActor>(BuildLayoutTransform(LayoutOrigin, LayoutRotation, SharedScreenOffset, 180.0f));
    SpawnIfMissing<AKemHostModeratorManagerActor>(GetActorTransform());

    bLocalLayoutInitialized = true;
    LastLayoutPawn = PlayerPawn;
    UE_LOG(LogTemp, Log, TEXT("KemSUinVR: Local layout refreshed around pawn %s at %s."),
        *PlayerPawn->GetName(),
        *LayoutOrigin.ToCompactString());
}

void AKemClassroomBootstrapActor::EnsurePlayerControlProxies()
{
    if (!HasAuthority() || !GetWorld())
    {
        return;
    }

    for (auto It = PlayerControlProxies.CreateIterator(); It; ++It)
    {
        if (!IsValid(It->Get()))
        {
            It.RemoveCurrent();
        }
    }

    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PlayerController = It->Get();
        if (!PlayerController)
        {
            continue;
        }

        const bool bHasProxy = PlayerControlProxies.ContainsByPredicate([PlayerController](const AKemPlayerControlProxy* Proxy)
        {
            return Proxy && Proxy->GetOwner() == PlayerController;
        });

        if (bHasProxy)
        {
            continue;
        }

        FActorSpawnParameters SpawnParameters;
        SpawnParameters.Owner = PlayerController;
        SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        if (AKemPlayerControlProxy* Proxy = GetWorld()->SpawnActor<AKemPlayerControlProxy>(SpawnParameters))
        {
            PlayerControlProxies.Add(Proxy);
            Proxy->ClientEvaluateLocalControlMode();
        }
    }
}

void AKemClassroomBootstrapActor::EnsureLocalPlayerControlMode()
{
    APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
    APawn* PlayerPawn = PlayerController ? PlayerController->GetPawn() : nullptr;
    if (!PlayerController || !PlayerPawn || PlayerPawn->IsA<AKemDesktopCharacter>() || !ShouldUseDesktopFallback())
    {
        return;
    }

    AKemPlayerControlProxy* LocalProxy = nullptr;
    for (const TObjectPtr<AKemPlayerControlProxy>& Proxy : PlayerControlProxies)
    {
        if (Proxy && Proxy->GetOwner() == PlayerController)
        {
            LocalProxy = Proxy;
            break;
        }
    }

    if (LocalProxy)
    {
        if (HasAuthority())
        {
            LocalProxy->ActivateDesktopModeForOwner();
        }
        else
        {
            LocalProxy->ServerRequestDesktopMode();
        }
    }
}

bool AKemClassroomBootstrapActor::ShouldUseDesktopFallback() const
{
    if (!GEngine || !GEngine->XRSystem.IsValid())
    {
        return true;
    }

    const bool bStereoRenderingActive = GEngine->IsStereoscopic3D();
    const bool bTrackedHMDPresent = GEngine->XRSystem->CountTrackedDevices(EXRTrackedDeviceType::HeadMountedDisplay) > 0;
    return !(bStereoRenderingActive && bTrackedHMDPresent);
}

FTransform AKemClassroomBootstrapActor::BuildLayoutTransform(const FVector& LayoutOrigin, const FRotator& LayoutRotation, const FVector& RelativeOffset, float AdditionalYaw) const
{
    const FTransform LayoutTransform(LayoutRotation, LayoutOrigin, FVector::OneVector);
    const FVector WorldLocation = LayoutTransform.TransformPosition(RelativeOffset);
    FRotator WorldRotation = LayoutRotation;
    WorldRotation.Yaw += AdditionalYaw;
    return FTransform(WorldRotation, WorldLocation, FVector::OneVector);
}

template <typename TActorClass>
TActorClass* AKemClassroomBootstrapActor::SpawnIfMissing(const FTransform& SpawnTransform)
{
    for (TActorIterator<TActorClass> It(GetWorld()); It; ++It)
    {
        It->SetActorTransform(SpawnTransform);
        return *It;
    }

    FActorSpawnParameters SpawnParameters;
    SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    return GetWorld()->SpawnActor<TActorClass>(TActorClass::StaticClass(), SpawnTransform, SpawnParameters);
}
