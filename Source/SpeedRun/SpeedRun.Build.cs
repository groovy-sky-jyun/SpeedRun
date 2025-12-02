// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SpeedRun : ModuleRules
{
	public SpeedRun(ReadOnlyTargetRules Target) : base(Target)
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
			"SpeedRun",
			"SpeedRun/Variant_Platforming",
			"SpeedRun/Variant_Platforming/Animation",
			"SpeedRun/Variant_Combat",
			"SpeedRun/Variant_Combat/AI",
			"SpeedRun/Variant_Combat/Animation",
			"SpeedRun/Variant_Combat/Gameplay",
			"SpeedRun/Variant_Combat/Interfaces",
			"SpeedRun/Variant_Combat/UI",
			"SpeedRun/Variant_SideScrolling",
			"SpeedRun/Variant_SideScrolling/AI",
			"SpeedRun/Variant_SideScrolling/Gameplay",
			"SpeedRun/Variant_SideScrolling/Interfaces",
			"SpeedRun/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
