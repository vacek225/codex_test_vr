#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KemPlayerControlProxy.generated.h"

UCLASS()
class KEMSUINVR_API AKemPlayerControlProxy : public AActor
{
    GENERATED_BODY()

public:
    AKemPlayerControlProxy();

    void ActivateDesktopModeForOwner();

    UFUNCTION(Client, Reliable)
    void ClientEvaluateLocalControlMode();

    UFUNCTION(Server, Reliable)
    void ServerRequestDesktopMode();
};
