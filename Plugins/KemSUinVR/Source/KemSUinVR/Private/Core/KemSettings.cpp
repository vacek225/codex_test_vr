#include "Core/KemSettings.h"

UKemSUinVRSettings::UKemSUinVRSettings()
{
    if (Auditoriums.IsEmpty())
    {
        FKemAuditoriumDefinition DefaultAuditorium;
        DefaultAuditorium.AuditoriumId = TEXT("MainHall");
        DefaultAuditorium.DisplayName = FText::FromString(TEXT("Main Hall"));
        DefaultAuditorium.MapPath = TEXT("/Game/VRTemplate/VRTemplateMap");
        DefaultAuditorium.Description = FText::FromString(TEXT("Default auditorium bound to the VRTemplate map for the prototype."));
        Auditoriums.Add(DefaultAuditorium);
    }
}

const FKemAuditoriumDefinition* UKemSUinVRSettings::FindAuditoriumById(FName AuditoriumId) const
{
    return Auditoriums.FindByPredicate([AuditoriumId](const FKemAuditoriumDefinition& Auditorium)
    {
        return Auditorium.AuditoriumId == AuditoriumId;
    });
}

FName UKemSUinVRSettings::GetCategoryName() const
{
    return TEXT("Plugins");
}
