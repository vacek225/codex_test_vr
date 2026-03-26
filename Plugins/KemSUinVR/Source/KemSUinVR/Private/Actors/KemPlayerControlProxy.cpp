#include "Actors/KemPlayerControlProxy.h"

#include "Actors/KemDesktopCharacter.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "IXRTrackingSystem.h"

AKemPlayerControlProxy::AKemPlayerControlProxy()
{
    bReplicates = true;
    bOnlyRelevantToOwner = true;
    SetReplicateMovement(false);
}

void AKemPlayerControlProxy::ActivateDesktopModeForOwner()
{
    APlayerController* PlayerController = Cast<APlayerController>(GetOwner());
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Warning, TEXT("KemSUinVR: Desktop mode activation skipped because owner is not a player controller."));
        return;
    }

    if (PlayerController->GetPawn() && PlayerController->GetPawn()->IsA<AKemDesktopCharacter>())
    {
        UE_LOG(LogTemp, Log, TEXT("KemSUinVR: Desktop character already active for %s."), *PlayerController->GetName());
        return;
    }

    APawn* ExistingPawn = PlayerController->GetPawn();
    const FVector SpawnLocation = ExistingPawn ? ExistingPawn->GetActorLocation() : GetActorLocation();
    const FRotator SpawnRotation = PlayerController->GetControlRotation();

    FActorSpawnParameters SpawnParameters;
    SpawnParameters.Owner = PlayerController;
    SpawnParameters.Instigator = ExistingPawn;
    SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AKemDesktopCharacter* DesktopCharacter = GetWorld()->SpawnActor<AKemDesktopCharacter>(
        AKemDesktopCharacter::StaticClass(),
        SpawnLocation,
        FRotator(0.0f, SpawnRotation.Yaw, 0.0f),
        SpawnParameters);
    if (!DesktopCharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("KemSUinVR: Failed to spawn desktop fallback character."));
        return;
    }

    PlayerController->Possess(DesktopCharacter);
    UE_LOG(LogTemp, Log, TEXT("KemSUinVR: Desktop fallback character possessed by %s at %s."),
        *PlayerController->GetName(),
        *DesktopCharacter->GetActorLocation().ToCompactString());

    if (ExistingPawn)
    {
        ExistingPawn->Destroy();
    }
}

void AKemPlayerControlProxy::ClientEvaluateLocalControlMode_Implementation()
{
    bool bDesktopFallbackRequested = true;
    if (GEngine && GEngine->XRSystem.IsValid())
    {
        const bool bStereoRenderingActive = GEngine->IsStereoscopic3D();
        const bool bTrackedHMDPresent = GEngine->XRSystem->CountTrackedDevices(EXRTrackedDeviceType::HeadMountedDisplay) > 0;
        bDesktopFallbackRequested = !(bStereoRenderingActive && bTrackedHMDPresent);
    }

    if (bDesktopFallbackRequested && GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            4.0f,
            FColor(24, 205, 255),
            TEXT("KemSUinVR: Desktop fallback enabled"));
    }

    UE_LOG(LogTemp, Log, TEXT("KemSUinVR: Local control mode evaluated. Desktop fallback requested = %s."),
        bDesktopFallbackRequested ? TEXT("true") : TEXT("false"));

    if (bDesktopFallbackRequested)
    {
        ServerRequestDesktopMode();
    }
}

void AKemPlayerControlProxy::ServerRequestDesktopMode_Implementation()
{
    ActivateDesktopModeForOwner();
}
