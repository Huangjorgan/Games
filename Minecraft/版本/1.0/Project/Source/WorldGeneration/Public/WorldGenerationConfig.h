
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WorldGenerationConfig.generated.h"

/**
 * 世界数据资产（DataAsset）
 * FWorldGenParams - 世界生成的核心参数
 * 可在编辑器中调整，实现不同风格的世界
 */
USTRUCT(BlueprintType)
struct FWorldGenParams
{
    GENERATED_BODY()

    /** 随机种子，决定世界唯一性 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Seed")
    int32 Seed = 12345;

    /** 地形噪声缩放系数，值越小地形越平滑 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain", meta = (ClampMin = "0.001"))
    float TerrainScale = 0.03f;

    /** 地形最大高度（单位：方块数） */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain", meta = (ClampMin = "1"))
    float HeightMultiplier = 10.0f;

    /** 世界总高度（Y 轴方块数），必须 >= HeightMultiplier */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World", meta = (ClampMin = "16"))
    int32 WorldHeight = 16;

    /** 区块横向尺寸（必须与 ChunkActor::SizeX/SizeZ 一致！） */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chunk", meta = (ClampMin = "1"))
    int32 ChunkSize = 16;
};


/**
 * UWorldGenerationConfig - 可在内容浏览器中创建的配置资源
 * 继承自 UPrimaryDataAsset，支持 Asset Manager 系统
 */
UCLASS()
class WORLDGENERATION_API UWorldGenerationConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
    /** 生成参数集合 */
    UPROPERTY(EditAnywhere, Category = "World Generation")
    FWorldGenParams Params;
};
