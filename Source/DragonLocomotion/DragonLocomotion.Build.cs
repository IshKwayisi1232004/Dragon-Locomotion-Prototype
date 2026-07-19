// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class DragonLocomotion : ModuleRules
{
	public DragonLocomotion(ReadOnlyTargetRules Target) : base(Target)
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
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"DragonLocomotion",
			"DragonLocomotion/Variant_Platforming",
			"DragonLocomotion/Variant_Platforming/Animation",
			"DragonLocomotion/Variant_Combat",
			"DragonLocomotion/Variant_Combat/AI",
			"DragonLocomotion/Variant_Combat/Animation",
			"DragonLocomotion/Variant_Combat/Gameplay",
			"DragonLocomotion/Variant_Combat/Interfaces",
			"DragonLocomotion/Variant_Combat/UI",
			"DragonLocomotion/Variant_SideScrolling",
			"DragonLocomotion/Variant_SideScrolling/AI",
			"DragonLocomotion/Variant_SideScrolling/Gameplay",
			"DragonLocomotion/Variant_SideScrolling/Interfaces",
			"DragonLocomotion/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
