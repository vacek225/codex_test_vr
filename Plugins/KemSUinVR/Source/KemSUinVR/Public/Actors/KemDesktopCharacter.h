#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "KemDesktopCharacter.generated.h"

class UCameraComponent;
class UWidgetInteractionComponent;
class UKemHubMenuWidget;

UCLASS()
class KEMSUINVR_API AKemDesktopCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AKemDesktopCharacter();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

protected:
    void UpdateDesktopMovement(float DeltaSeconds);
    void UpdateDesktopInteraction();
    void EnsureDesktopOverlayVisibility();
    void ApplyDesktopMenuInputMode() const;
    void ApplyDesktopGameplayInputMode() const;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "KemSUinVR")
    TObjectPtr<UCameraComponent> CameraComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "KemSUinVR")
    TObjectPtr<UWidgetInteractionComponent> WidgetInteraction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "KemSUinVR|Desktop")
    float DesktopMoveSpeed = 700.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "KemSUinVR|Desktop")
    float DesktopLookSensitivity = 0.11f;

private:
    bool bWasPrimaryMouseButtonDown = false;
    bool bDesktopMenuVisible = false;

    UPROPERTY(Transient)
    TObjectPtr<UKemHubMenuWidget> DesktopHubMenuWidget;
};
