
// VoxelPersistenceSubsystem.cpp
#include "VoxelPersistenceSubsystem.h"
#include "VoxelPersistenceJsonUtils.h"
#include "VoxelPersistencePaths.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Async/Async.h"
#include "LogVoxelPersistence.h"

bool UVoxelPersistenceSubsystem::DoesWorldExist(const FString& WorldName)
{
    if (WorldName.IsEmpty()) return false;
    FString MetaPath = FVoxelPersistencePaths::GetMetaFilePath(WorldName);
    return FPaths::FileExists(MetaPath);
}

void UVoxelPersistenceSubsystem::SaveWorldAsync(
    const FString& WorldName,
    const FVoxelWorldMeta& Meta,
    const TMap<FIntPoint, FVoxelChunkData>& ModifiedChunks,
    const FOnVoxelWorldSaved& OnComplete)
{
    if (WorldName.IsEmpty())
    {
        UE_LOG(H_LogVoxelPersistence, Error, TEXT("SaveWorldAsync: WorldName is empty!"));
        if (OnComplete.IsBound()) AsyncTask(ENamedThreads::GameThread, [OnComplete]() { OnComplete.Execute(false); });
        return;
    }

    FString WorldNameCopy = WorldName;
    FVoxelWorldMeta MetaCopy = Meta;
    TMap<FIntPoint, FVoxelChunkData> ChunksCopy = ModifiedChunks;

    AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [WorldNameCopy, MetaCopy, ChunksCopy, OnComplete]()
        {
            bool bSuccess = SaveWorld_Internal(WorldNameCopy, MetaCopy, ChunksCopy);

            AsyncTask(ENamedThreads::GameThread, [OnComplete, bSuccess]()
                {
                    if (OnComplete.IsBound())
                        OnComplete.Execute(bSuccess);
                });
        });
}

void UVoxelPersistenceSubsystem::LoadWorldMetaAsync(
    const FString& WorldName,
    const FOnVoxelWorldLoaded& OnComplete)
{
    if (WorldName.IsEmpty())
    {
        UE_LOG(H_LogVoxelPersistence, Error, TEXT("LoadWorldMetaAsync: WorldName is empty!"));
        if (OnComplete.IsBound()) AsyncTask(ENamedThreads::GameThread, [OnComplete]() { OnComplete.Execute(false, FVoxelWorldMeta()); });
        return;
    }

    FString WorldNameCopy = WorldName;

    AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [WorldNameCopy, OnComplete]()
        {
            FVoxelWorldMeta LoadedMeta;
            bool bSuccess = false;

            FString MetaPath = FVoxelPersistencePaths::GetMetaFilePath(WorldNameCopy);
            TSharedPtr<FJsonObject> Json;
            if (FVoxelPersistenceJsonUtils::LoadJsonFromFile(MetaPath, Json))
            {
                bSuccess = FVoxelPersistenceJsonUtils::FromJson(Json, LoadedMeta);
            }

            AsyncTask(ENamedThreads::GameThread, [OnComplete, bSuccess, LoadedMeta]()
                {
                    if (OnComplete.IsBound())
                        OnComplete.Execute(bSuccess, LoadedMeta);
                });
        });
}

bool UVoxelPersistenceSubsystem::LoadChunkSync(const FString& WorldName, const FIntPoint& ChunkPos, FVoxelChunkData& OutChunk)
{
    if (WorldName.IsEmpty()) return false;

    FString ChunkPath = FVoxelPersistencePaths::GetChunkFilePath(WorldName, ChunkPos);
    TSharedPtr<FJsonObject> Json;
    if (!FVoxelPersistenceJsonUtils::LoadJsonFromFile(ChunkPath, Json))
        return false;

    return FVoxelPersistenceJsonUtils::FromJson(Json, OutChunk);
}

// --- Internal: Save on background thread ---
bool UVoxelPersistenceSubsystem::SaveWorld_Internal(
    const FString& WorldName,
    const FVoxelWorldMeta& Meta,
    const TMap<FIntPoint, FVoxelChunkData>& ModifiedChunks)
{
    // Save meta
    FString MetaPath = FVoxelPersistencePaths::GetMetaFilePath(WorldName);
    TSharedPtr<FJsonObject> MetaJson = FVoxelPersistenceJsonUtils::ToJson(Meta);
    if (!FVoxelPersistenceJsonUtils::SaveJsonToFile(MetaPath, MetaJson))
    {
        UE_LOG(H_LogVoxelPersistence, Error, TEXT("Failed to save meta file for world: %s"), *WorldName);
        return false;
    }

    // Save chunks
    for (const auto& Pair : ModifiedChunks)
    {
        FString ChunkPath = FVoxelPersistencePaths::GetChunkFilePath(WorldName, Pair.Key);
        TSharedPtr<FJsonObject> ChunkJson = FVoxelPersistenceJsonUtils::ToJson(Pair.Value);
        if (!FVoxelPersistenceJsonUtils::SaveJsonToFile(ChunkPath, ChunkJson))
        {
            UE_LOG(H_LogVoxelPersistence, Error, TEXT("Failed to save chunk %d,%d for world: %s"), Pair.Key.X, Pair.Key.Y, *WorldName);
            return false;
        }
    }

    UE_LOG(H_LogVoxelPersistence, Log, TEXT("Successfully saved world: %s (chunks: %d)"), *WorldName, ModifiedChunks.Num());
    return true;
}