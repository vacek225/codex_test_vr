#include "Actors/KemDesktopCharacter.h"

#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetInteractionComponent.h"
#include "Core/KemRoomSubsystem.h"
#include "Engine/Engine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "UI/KemWidgets.h"

AKemDesktopCharacter::AKemDesktopCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    bUseControllerRotationYaw = true;

    GetCapsuleComponent()->InitCapsuleSize(34.0f, 88.0f);
    GetCharacterMovement()->bOrientRotationToMovement = false;
    GetCharacterMovement()->MaxFlySpeed = DesktopMoveSpeed;
    GetCharacterMovement()->BrakingDecelerationFlying = 4096.0f;
    GetCharacterMovement()->AirControl = 1.0f;
    GetCharacterMovement()->GravityScale = 0.0f;

    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("DesktopCamera"));
    CameraComponent->SetupAttachment(GetCapsuleComponent());
    CameraComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 64.0f));
    CameraComponent->bUsePawnControlRotation = true;

    WidgetInteraction = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("WidgetInteraction"));
    WidgetInteraction->SetupAttachment(CameraComponent);
    WidgetInteraction->InteractionSource = EWidgetInteractionSource::CenterScreen;
    WidgetInteraction->InteractionDistance = 2500.0f;
    WidgetInteraction->TraceChannel = ECC_Visibility;
    WidgetInteraction->bShowDebug = true;
    WidgetInteraction->DebugColor = FLinearColor(0.08f, 0.78f, 1.0f, 1.0f);
}

void AKemDesktopCharacter::BeginPlay()
{
    Super::BeginPlay();

    GetCharacterMovement()->SetMovementMode(MOVE_Flying);

    if (IsLocallyControlled())
    {
        UE_LOG(LogTemp, Log, TEXT("KemSUinVR: Desktop character BeginPlay at %s."), *GetActorLocation().ToCompactString());
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 6.0f, FColor(24, 205, 255), TEXT("KemSUinVR: Desktop character active"));
        }
        EnsureDesktopOverlayVisibility();
    }
}

void AKemDesktopCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!IsLocallyControlled())
    {
        return;
    }

    EnsureDesktopOverlayVisibility();
    if (bDesktopMenuVisible)
    {
        return;
    }

    UpdateDesktopMovement(DeltaSeconds);
    UpdateDesktopInteraction();
}

void AKemDesktopCharacter::UpdateDesktopMovement(float DeltaSeconds)
{
    APlayerController* PlayerController = Cast<APlayerController>(GetController());
    if (!PlayerController)
    {
        return;
    }

    const float ForwardAxis = (PlayerController->IsInputKeyDown(EKeys::W) ? 1.0f : 0.0f) - (PlayerController->IsInputKeyDown(EKeys::S) ? 1.0f : 0.0f);
    const float RightAxis = (PlayerController->IsInputKeyDown(EKeys::D) ? 1.0f : 0.0f) - (PlayerController->IsInputKeyDown(EKeys::A) ? 1.0f : 0.0f);
    const float UpAxis = (PlayerController->IsInputKeyDown(EKeys::E) ? 1.0f : 0.0f) + (PlayerController->IsInputKeyDown(EKeys::SpaceBar) ? 1.0f : 0.0f)
        - (PlayerController->IsInputKeyDown(EKeys::Q) ? 1.0f : 0.0f) - (PlayerController->IsInputKeyDown(EKeys::LeftControl) ? 1.0f : 0.0f);

    if (!FMath::IsNearlyZero(ForwardAxis))
    {
        AddMovementInput(GetActorForwardVector(), ForwardAxis);
    }

    if (!FMath::IsNearlyZero(RightAxis))
    {
        AddMovementInput(GetActorRightVector(), RightAxis);
    }

    if (!FMath::IsNearlyZero(UpAxis))
    {
        AddMovementInput(FVector::UpVector, UpAxis);
    }

    float MouseDeltaX = 0.0f;
    float MouseDeltaY = 0.0f;
    PlayerController->GetInputMouseDelta(MouseDeltaX, MouseDeltaY);

    if (!FMath::IsNearlyZero(MouseDeltaX))
    {
        AddControllerYawInput(MouseDeltaX * DesktopLookSensitivity);
    }

    if (!FMath::IsNearlyZero(MouseDeltaY))
    {
        AddControllerPitchInput(MouseDeltaY * -DesktopLookSensitivity);
    }
}

void AKemDesktopCharacter::UpdateDesktopInteraction()
{
    APlayerController* PlayerController = Cast<APlayerController>(GetController());
    if (!PlayerController || !WidgetInteraction)
    {
        return;
    }

    const bool bPrimaryMouseButtonDown = PlayerController->IsInputKeyDown(EKeys::LeftMouseButton);
    if (bPrimaryMouseButtonDown && !bWasPrimaryMouseButtonDown)
    {
        WidgetInteraction->PressPointerKey(EKeys::LeftMouseButton);
    }
    else if (!bPrimaryMouseButtonDown && bWasPrimaryMouseButtonDown)
    {
        WidgetInteraction->ReleasePointerKey(EKeys::LeftMouseButton);
    }

    bWasPrimaryMouseButtonDown = bPrimaryMouseButtonDown;
}

void AKemDesktopCharacter::EnsureDesktopOverlayVisibility()
{
    APlayerController* PlayerController = Cast<APlayerController>(GetController());
    UKemRoomSubsystem* RoomSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UKemRoomSubsystem>() : nullptr;
    if (!PlayerController || !RoomSubsystem)
    {
        return;
    }

    const bool bShouldShowMenu = !RoomSubsystem->GetCurrentRoomInfo().bIsInRoom;
    if (bShouldShowMenu)
    {
        if (!DesktopHubMenuWidget)
        {
            DesktopHubMenuWidget = CreateWidget<UKemHubMenuWidget>(PlayerController, UKemHubMenuWidget::StaticClass());
            if (DesktopHubMenuWidget)
            {
                DesktopHubMenuWidget->AddToViewport(50);
                DesktopHubMenuWidget->SetAlignmentInViewport(FVector2D(0.5f, 0.5f));
                int32 ViewportX = 0;
                int32 ViewportY = 0;
                PlayerController->GetViewportSize(ViewportX, ViewportY);
                DesktopHubMenuWidget->SetPositionInViewport(FVector2D(ViewportX * 0.5f, ViewportY * 0.5f), true);
                DesktopHubMenuWidget->SetDesiredSizeInViewport(FVector2D(760.0f, 920.0f));
                UE_LOG(LogTemp, Log, TEXT("KemSUinVR: Desktop viewport menu created."));
            }
        }

        if (!bDesktopMenuVisible)
        {
            if (DesktopHubMenuWidget)
            {
                DesktopHubMenuWidget->SetVisibility(ESlateVisibility::Visible);
            }

            bWasPrimaryMouseButtonDown = false;
            bDesktopMenuVisible = true;
            ApplyDesktopMenuInputMode();
        }
        return;
    }

    if (bDesktopMenuVisible)
    {
        if (DesktopHubMenuWidget)
        {
            DesktopHubMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
            UE_LOG(LogTemp, Log, TEXT("KemSUinVR: Desktop viewport menu hidden after room entry."));
        }

        bWasPrimaryMouseButtonDown = false;
        bDesktopMenuVisible = false;
        ApplyDesktopGameplayInputMode();
    }
}

void AKemDesktopCharacter::ApplyDesktopMenuInputMode() const
{
    if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
    {
        if (WidgetInteraction)
        {
            WidgetInteraction->SetComponentTickEnabled(false);
            WidgetInteraction->bShowDebug = false;
        }

        PlayerController->SetShowMouseCursor(true);
        PlayerController->bEnableClickEvents = true;
        PlayerController->bEnableMouseOverEvents = true;
        PlayerController->SetIgnoreLookInput(true);
        PlayerController->SetIgnoreMoveInput(true);

        FInputModeUIOnly InputMode;
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        if (DesktopHubMenuWidget)
        {
            InputMode.SetWidgetToFocus(DesktopHubMenuWidget->TakeWidget());
        }
        PlayerController->SetInputMode(InputMode);
    }
}

void AKemDesktopCharacter::ApplyDesktopGameplayInputMode() const
{
    if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
    {
        if (WidgetInteraction)
        {
            WidgetInteraction->SetComponentTickEnabled(true);
            WidgetInteraction->bShowDebug = true;
        }

        PlayerController->SetShowMouseCursor(false);
        PlayerController->bEnableClickEvents = false;
        PlayerController->bEnableMouseOverEvents = false;
        PlayerController->SetIgnoreLookInput(false);
        PlayerController->SetIgnoreMoveInput(false);
        FInputModeGameOnly InputMode;
        PlayerController->SetInputMode(InputMode);
    }
}
