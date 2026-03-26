#include "Actors/KemWorldActors.h"

#include "Actors/KemClassroomStateActor.h"
#include "Components/WidgetComponent.h"
#include "Core/KemRoomSubsystem.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/Pawn.h"
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
    SetActorScale3D(FVector(0.0035f));
}

AKemRoomCodeDisplayActor::AKemRoomCodeDisplayActor()
{
    bFaceLocalPlayer = true;
    WidgetComponent->SetWidgetClass(UKemRoomCodeWidget::StaticClass());
    WidgetComponent->SetDrawSize(FVector2D(700.0f, 280.0f));
    SetActorScale3D(FVector(0.005f));
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
    WidgetComponent->SetWidgetClass(UKemTeacherControlWidget::StaticClass());
    WidgetComponent->SetDrawSize(FVector2D(900.0f, 1200.0f));
    SetActorScale3D(FVector(0.0035f));
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
    WidgetComponent->SetWidgetClass(UKemBrowserDeskWidget::StaticClass());
    WidgetComponent->SetDrawSize(FVector2D(1200.0f, 1100.0f));
    SetActorScale3D(FVector(0.0025f));
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
    WidgetComponent->SetWidgetClass(UKemLectureScreenWidget::StaticClass());
    WidgetComponent->SetDrawSize(FVector2D(1600.0f, 1100.0f));
    SetActorScale3D(FVector(0.004f));
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
    WidgetComponent->SetWidgetClass(UKemLectureScreenWidget::StaticClass());
    WidgetComponent->SetDrawSize(FVector2D(1200.0f, 850.0f));
    SetActorScale3D(FVector(0.003f));
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
    SetActorScale3D(FVector(0.0028f));
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
    PrimaryActorTick.bCanEverTick = false;
}

void AKemClassroomBootstrapActor::BeginPlay()
{
    Super::BeginPlay();

    const FTransform ActorTransform = GetActorTransform();

    SpawnIfMissing<AKemHubActor>(FTransform(ActorTransform.GetRotation(), ActorTransform.TransformPosition(HubOffset), ActorTransform.GetScale3D()));
    SpawnIfMissing<AKemRoomCodeDisplayActor>(FTransform(ActorTransform.GetRotation(), ActorTransform.TransformPosition(RoomCodeOffset), ActorTransform.GetScale3D()));
    SpawnIfMissing<AKemTeacherControlPanelActor>(FTransform(ActorTransform.GetRotation(), ActorTransform.TransformPosition(TeacherPanelOffset), ActorTransform.GetScale3D()));
    SpawnIfMissing<AKemBrowserDeskActor>(FTransform(ActorTransform.GetRotation(), ActorTransform.TransformPosition(BrowserDeskOffset), ActorTransform.GetScale3D()));

    FRotator ScreenRotation = ActorTransform.GetRotation().Rotator();
    ScreenRotation.Yaw += 180.0f;
    SpawnIfMissing<AKemSharedLectureScreenActor>(FTransform(ScreenRotation, ActorTransform.TransformPosition(SharedScreenOffset), ActorTransform.GetScale3D()));
    SpawnIfMissing<AKemHostModeratorManagerActor>(ActorTransform);
}

template <typename TActorClass>
TActorClass* AKemClassroomBootstrapActor::SpawnIfMissing(const FTransform& SpawnTransform)
{
    for (TActorIterator<TActorClass> It(GetWorld()); It; ++It)
    {
        return *It;
    }

    FActorSpawnParameters SpawnParameters;
    SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    return GetWorld()->SpawnActor<TActorClass>(TActorClass::StaticClass(), SpawnTransform, SpawnParameters);
}
