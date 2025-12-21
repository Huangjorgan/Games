
#include "VoxelPersistencePaths.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"

FString FVoxelPersistencePaths::GetWorldSaveDir(const FString& WorldName)
{
    return FPaths::ProjectSavedDir() / TEXT("VoxelWorlds") / WorldName;
}

FString FVoxelPersistencePaths::GetMetaFilePath(const FString& WorldName)
{
    return GetWorldSaveDir(WorldName) / TEXT("world_meta.json");
}

FString FVoxelPersistencePaths::GetChunkDir(const FString& WorldName)
{
    return GetWorldSaveDir(WorldName) / TEXT("chunks");
}

FString FVoxelPersistencePaths::GetChunkFilePath(const FString& WorldName, const FIntPoint& ChunkPos)
{
    return GetChunkDir(WorldName) / FString::Printf(TEXT("chunk_%d_%d.json"), ChunkPos.X, ChunkPos.Y);
}