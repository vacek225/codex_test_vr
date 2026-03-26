#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/KemTypes.h"
#include "KemWorldActors.generated.h"

class UKemParticipantModeratorWidget;
class UWidgetComponent;
class AKemPlayerControlProxy;
class APlayerController;
class APawn;

UCLASS(Abstract)
class KEMSUINVR_API AKemWidgetPanelActorBase : public AActor
{
    GENERATED_BODY()

public:
    AKemWidgetPanelActorBase();
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "KemSUinVR")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "KemSUinVR")
    TObjectPtr<UWidgetComponent> WidgetComponent;

protected:
    virtual void BeginPlay() override;
    void FaceFirstPlayer();

    UPROPERTY(EditAnywhere, Category = "KemSUinVR")
    bool bFaceLocalPlayer = false;
};

UCLASS()
class KEMSUINVR_API AKemHubActor : public AKemWidgetPanelActorBase
{
    GENERATED_BODY()

public:
    AKemHubActor();
    virtual void Tick(float DeltaSeconds) override;
};

UCLASS()
class KEMSUINVR_API AKemRoomCodeDisplayActor : public AKemWidgetPanelActorBase
{
    GENERATED_BODY()

public:
    AKemRoomCodeDisplayActor();
    virtual void Tick(float DeltaSeconds) override;
};

UCLASS()
class KEMSUINVR_API AKemTeacherControlPanelActor : public AKemWidgetPanelActorBase
{
    GENERATED_BODY()

public:
    AKemTeacherControlPanelActor();
    virtual void Tick(float DeltaSeconds) override;
};

UCLASS()
class KEMSUINVR_API AKemBrowserDeskActor : public AKemWidgetPanelActorBase
{
    GENERATED_BODY()

public:
    AKemBrowserDeskActor();
    virtual void Tick(float DeltaSeconds) override;
};

UCLASS()
class KEMSUINVR_API AKemSharedLectureScreenActor : public AKemWidgetPanelActorBase
{
    GENERATED_BODY()

public:
    AKemSharedLectureScreenActor();

protected:
    virtual void BeginPlay() override;
};

UCLASS()
class KEMSUINVR_API AKemLocalLectureScreenActor : public AKemWidgetPanelActorBase
{
    GENERATED_BODY()

public:
    AKemLocalLectureScreenActor();

protected:
    virtual void BeginPlay() override;
};

UCLASS()
class KEMSUINVR_API AKemParticipantModeratorActor : public AKemWidgetPanelActorBase
{
    GENERATED_BODY()

public:
    AKemParticipantModeratorActor();
    virtual void Tick(float DeltaSeconds) override;

    void BindToParticipant(APawn* TargetPawn, const FKemParticipantInfo& ParticipantInfo);
    bool IsBoundToPawn(const APawn* Pawn) const;

private:
    TWeakObjectPtr<APawn> ObservedPawn;
    FKemParticipantInfo CachedParticipantInfo;
};

UCLASS()
class KEMSUINVR_API AKemHostModeratorManagerActor : public AActor
{
    GENERATED_BODY()

public:
    AKemHostModeratorManagerActor();
    virtual void Tick(float DeltaSeconds) override;

private:
    TMap<TWeakObjectPtr<APawn>, TObjectPtr<AKemParticipantModeratorActor>> ModeratorPanels;
    float RefreshAccumulator = 0.0f;
};

UCLASS()
class KEMSUINVR_API AKemClassroomBootstrapActor : public AActor
{
    GENERATED_BODY()

public:
    AKemClassroomBootstrapActor();
    virtual void Tick(float DeltaSeconds) override;

protected:
    virtual void BeginPlay() override;

private:
    void EnsureLocalLayoutInitialized();
    void EnsurePlayerControlProxies();
    void EnsureLocalPlayerControlMode();
    bool ShouldUseDesktopFallback() const;
    FTransform BuildLayoutTransform(const FVector& LayoutOrigin, const FRotator& LayoutRotation, const FVector& RelativeOffset, float AdditionalYaw = 0.0f) const;

    template <typename TActorClass>
    TActorClass* SpawnIfMissing(const FTransform& SpawnTransform);

private:
    UPROPERTY(EditAnywhere, Category = "KemSUinVR")
    FVector HubOffset = FVector(180.0f, 0.0f, 120.0f);

    UPROPERTY(EditAnywhere, Category = "KemSUinVR")
    FVector RoomCodeOffset = FVector(220.0f, -120.0f, 175.0f);

    UPROPERTY(EditAnywhere, Category = "KemSUinVR")
    FVector TeacherPanelOffset = FVector(210.0f, 130.0f, 110.0f);

    UPROPERTY(EditAnywhere, Category = "KemSUinVR")
    FVector BrowserDeskOffset = FVector(210.0f, -140.0f, 105.0f);

    UPROPERTY(EditAnywhere, Category = "KemSUinVR")
    FVector SharedScreenOffset = FVector(460.0f, 0.0f, 190.0f);

    bool bLocalLayoutInitialized = false;
    float ProxyRefreshAccumulator = 0.0f;
    TWeakObjectPtr<APawn> LastLayoutPawn;

    UPROPERTY(Transient)
    TArray<TObjectPtr<AKemPlayerControlProxy>> PlayerControlProxies;
};
