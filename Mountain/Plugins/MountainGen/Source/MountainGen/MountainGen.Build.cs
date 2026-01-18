using UnrealBuildTool;

public class MountainGen : ModuleRules
{
    public MountainGen(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "ProceduralMeshComponent"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
            }
        );
    }
}