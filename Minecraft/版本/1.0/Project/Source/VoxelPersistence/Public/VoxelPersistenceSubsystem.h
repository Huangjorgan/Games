
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "VoxelPersistenceTypes.h"
#include "VoxelPersistenceSubsystem.generated.h"

/**
 * 存档系统核心子系统
 */

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnVoxelWorldSaved, bool, bSuccess);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnVoxelWorldLoaded, bool, bSuccess, FVoxelWorldMeta, Meta);

UCLASS()
class VOXELPERSISTENCE_API UVoxelPersistenceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
    // 异步保存整个世界
    UFUNCTION(BlueprintCallable, Category = "Voxel Persistence")
    void SaveWorldAsync(
        const FString& WorldName,
        const FVoxelWorldMeta& Meta,
        const TMap<FIntPoint, FVoxelChunkData>& ModifiedChunks,
        const FOnVoxelWorldSaved& OnComplete
    );

    // 异步加载世界元数据（不加载区块）
    UFUNCTION(BlueprintCallable, Category = "Voxel Persistence")
    void LoadWorldMetaAsync(
        const FString& WorldName,
        const FOnVoxelWorldLoaded& OnComplete
    );

    // 同步加载单个区块（通常在生成时调用）
    UFUNCTION(BlueprintCallable, Category = "Voxel Persistence")
    bool LoadChunkSync(const FString& WorldName, const FIntPoint& ChunkPos, FVoxelChunkData& OutChunk);

    // 工具函数
    UFUNCTION(BlueprintPure, Category = "Voxel Persistence")
    static bool DoesWorldExist(const FString& WorldName);

private:
    // 后台线程任务
    static bool SaveWorld_Internal(
        const FString& WorldName,
        const FVoxelWorldMeta& Meta,
        const TMap<FIntPoint, FVoxelChunkData>& ModifiedChunks
    );
};
