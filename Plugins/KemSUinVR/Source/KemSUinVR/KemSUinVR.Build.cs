using UnrealBuildTool;

public class KemSUinVR : ModuleRules
{
    public KemSUinVR(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "InputCore",
                "UMG",
                "Slate",
                "SlateCore",
                "NetCore",
                "Networking",
                "Sockets",
                "OnlineSubsystem",
                "OnlineSubsystemUtils",
                "AudioMixer",
                "GameplayTags",
                "DeveloperSettings",
                "WebBrowserWidget"
            });

        PrivateDependencyModuleNames.AddRange(
            new[]
            {
                "Projects",
                "ApplicationCore",
                "HTTP"
            });
    }
}
