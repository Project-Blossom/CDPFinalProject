// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Prototype : ModuleRules
{
	public Prototype(ReadOnlyTargetRules Target) : base(Target)
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
            "ProceduralMeshComponent",
            "PhysicsCore"
        });

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"Prototype",
			"Prototype/Variant_Platforming",
			"Prototype/Variant_Platforming/Animation",
			"Prototype/Variant_Combat",
			"Prototype/Variant_Combat/AI",
			"Prototype/Variant_Combat/Animation",
			"Prototype/Variant_Combat/Gameplay",
			"Prototype/Variant_Combat/Interfaces",
			"Prototype/Variant_Combat/UI",
			"Prototype/Variant_SideScrolling",
			"Prototype/Variant_SideScrolling/AI",
			"Prototype/Variant_SideScrolling/Gameplay",
			"Prototype/Variant_SideScrolling/Interfaces",
			"Prototype/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}