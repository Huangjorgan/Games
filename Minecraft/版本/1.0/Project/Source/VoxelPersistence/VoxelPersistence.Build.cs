using UnrealBuildTool;

public class VoxelPersistence: ModuleRules
{
    public VoxelPersistence(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivateDependencyModuleNames.AddRange(new string[] 
        {
            "Core",
            "CoreUObject", 
            "Engine",
            "Json",
            "JsonUtilities"
        }
        );
    }
}
