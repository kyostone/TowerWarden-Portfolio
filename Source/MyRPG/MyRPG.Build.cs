// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MyRPG : ModuleRules
{
	public MyRPG(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
            "GameplayAbilities",
			"GameplayTags",
			"GameplayTasks",
            "Niagara"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
			"OnlineSubsystem",
			"OnlineSubsystemUtils"
        });

        DynamicallyLoadedModuleNames.Add("OnlineSubsystemNull");

        PublicIncludePaths.AddRange(new string[] {
			"MyRPG",
			"MyRPG/Variant_Platforming",
			"MyRPG/Variant_Platforming/Animation",
			"MyRPG/Variant_Combat",
			"MyRPG/Variant_Combat/AI",
			"MyRPG/Variant_Combat/Animation",
			"MyRPG/Variant_Combat/Gameplay",
			"MyRPG/Variant_Combat/Interfaces",
			"MyRPG/Variant_Combat/UI",
			"MyRPG/Variant_SideScrolling",
			"MyRPG/Variant_SideScrolling/AI",
			"MyRPG/Variant_SideScrolling/Gameplay",
			"MyRPG/Variant_SideScrolling/Interfaces",
			"MyRPG/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
