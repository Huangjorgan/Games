using UnrealBuildTool;

public class ChunkBlock: ModuleRules
{
    public ChunkBlock(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivateDependencyModuleNames.AddRange(new string[] {
            "Core", 
            "CoreUObject",
            "Engine"});
    }
}
