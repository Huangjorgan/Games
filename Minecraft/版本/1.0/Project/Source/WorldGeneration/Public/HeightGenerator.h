#pragma once

#include "CoreMinimal.h"
#include "HeightGenerator.generated.h"


struct FWorldGenParams;
class FastNoise;

/**
 * @brief 高度生成器（静态工具类）
 *
 * 使用 FastNoise 库生成程序化地形高度。
 * 所有方法均为静态，线程安全（通过临界区保护噪声实例缓存）。
 */
UCLASS()
class UHeightGenerator : public UObject
{
    GENERATED_BODY()

public:
    /**
     * @brief 根据世界坐标生成单点高度
     *
     * 输入为水平坐标 (X, Y)，输出为垂直高度 Z。
     *
     * @param WorldX 世界 X 坐标（水平）
     * @param WorldY 世界 Y 坐标（水平）
     * @param Params 生成参数（种子、缩放等）
     * @return 高度值（Z 坐标），范围 [0, WorldHeight-1]
     */
    static int32 GenerateHeightAt(float WorldX, float WorldY, const FWorldGenParams& Params);

    /**
     * @brief 生成整个区块（16x16）的地表高度图
     *
     * 输出数组按行主序存储：OutHeights[x + y * 16]
     *
     * @param ChunkX 区块 X 索引
     * @param ChunkY 区块 Y 索引
     * @param Params 生成参数
     * @param OutHeights 输出高度数组（大小 256）
     */
    static void GenerateChunkHeights(
        int32 ChunkX,
        int32 ChunkY,
        const FWorldGenParams& Params,
        TArray<int32>& OutHeights
    );

private:
    /** 噪声实例缓存（按种子分组） */
    static TMap<int32, FastNoise*> GNoiseCache;
    /** 保护缓存的临界区（确保线程安全） */
    static FCriticalSection GCriticalSection;
};