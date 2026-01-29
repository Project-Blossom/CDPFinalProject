// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class b1234 : ModuleRules
{
	public b1234(ReadOnlyTargetRules Target) : base(Target)
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
			"b1234",
			"b1234/Variant_Platforming",
			"b1234/Variant_Platforming/Animation",
			"b1234/Variant_Combat",
			"b1234/Variant_Combat/AI",
			"b1234/Variant_Combat/Animation",
			"b1234/Variant_Combat/Gameplay",
			"b1234/Variant_Combat/Interfaces",
			"b1234/Variant_Combat/UI",
			"b1234/Variant_SideScrolling",
			"b1234/Variant_SideScrolling/AI",
			"b1234/Variant_SideScrolling/Gameplay",
			"b1234/Variant_SideScrolling/Interfaces",
			"b1234/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
