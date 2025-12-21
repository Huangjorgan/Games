using UnrealBuildTool;

public class GameCore: ModuleRules
{
    public GameCore(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivateDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject", 
            "Engine",
            "WorldGeneration",
            "ChunkBlock",
            "VoxelPersistence"
        });
    }
}
