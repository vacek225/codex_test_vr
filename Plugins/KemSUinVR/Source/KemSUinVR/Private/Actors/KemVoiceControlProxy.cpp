#include "Actors/KemVoiceControlProxy.h"

#include "GameFramework/PlayerController.h"
#include "Kismet/KismetSystemLibrary.h"

AKemVoiceControlProxy::AKemVoiceControlProxy()
{
    bReplicates = true;
    bOnlyRelevantToOwner = true;
    SetReplicateMovement(false);
}

void AKemVoiceControlProxy::ClientSetVoiceTransmitEnabled_Implementation(bool bEnabled)
{
    ApplyVoiceTransmitEnabled(bEnabled);
}

void AKemVoiceControlProxy::ClientBootstrapVoice_Implementation()
{
    ApplyVoiceTransmitEnabled(true);
}

void AKemVoiceControlProxy::ApplyVoiceTransmitEnabled(bool bEnabled)
{
    if (APlayerController* PlayerController = Cast<APlayerController>(GetOwner()))
    {
        const FString Command = bEnabled ? TEXT("ToggleSpeaking 1") : TEXT("ToggleSpeaking 0");
        PlayerController->ConsoleCommand(Command, true);
    }
}
