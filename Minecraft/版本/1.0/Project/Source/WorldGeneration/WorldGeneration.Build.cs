using UnrealBuildTool;

public class WorldGeneration: ModuleRules
{
    public WorldGeneration(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivateDependencyModuleNames.AddRange(new string[] 
        {
            "Core",
            "CoreUObject",
            "Engine"
        }
        );
        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "ChunkBlock"   
        }
        );
    }
}
