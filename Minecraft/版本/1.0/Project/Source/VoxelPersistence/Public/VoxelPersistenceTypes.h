
#pragma once

#include "CoreMinimal.h"
#include "VoxelPersistenceTypes.generated.h"

/**
 * 存储结构定义
 * FVoxelWorldMeta - 存档元数据
 * FVoxelChunkData - 区块数据
 */
USTRUCT(BlueprintType)
struct FVoxelWorldMeta
{
	GENERATED_BODY()

    /*存档版本*/
    UPROPERTY()
    int32 Version = 1;

    /*存档种子*/
    UPROPERTY()
    int32 Seed = 0;

    /*存档名称*/
    UPROPERTY()
    FString WorldName;

	/*玩家位置*/
    UPROPERTY()
    FVector PlayerLocation = FVector::ZeroVector;

	/*玩家朝向*/
    UPROPERTY()
    FRotator PlayerRotation = FRotator::ZeroRotator;

	/*最后保存时间*/
    UPROPERTY()
    FString LastSavedTime;

	/*区块列表*/
    UPROPERTY()
    TArray<FIntPoint> ChunkList;
};

USTRUCT(BlueprintType)
struct FVoxelChunkData
{
    GENERATED_BODY()

    /*区块坐标*/
    UPROPERTY()
    FIntPoint ChunkCoordinate;

    /*体素数据*/
    UPROPERTY()
    TArray<int32> VoxelData;
};