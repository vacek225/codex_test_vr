#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Core/KemTypes.h"
#include "KemSettings.generated.h"

UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="KemSUinVR"))
class KEMSUINVR_API UKemSUinVRSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    UKemSUinVRSettings();

    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Session")
    int32 DefaultPublicConnections = 16;

    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Session")
    int32 RoomCodeLength = 6;

    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Session")
    FString DefaultLectureUrl = TEXT("https://kemsu.ru");

    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Session")
    TArray<FKemAuditoriumDefinition> Auditoriums;

    const FKemAuditoriumDefinition* FindAuditoriumById(FName AuditoriumId) const;

    virtual FName GetCategoryName() const override;
};
