#include "HeightGenerator.h"
#include "WorldGenerationConfig.h"
#include "FastNoise.h" // FastNoise 经典版

// 静态成员定义
TMap<int32, FastNoise*> UHeightGenerator::GNoiseCache;
FCriticalSection UHeightGenerator::GCriticalSection;

int32 UHeightGenerator::GenerateHeightAt(float WorldX, float WorldY, const FWorldGenParams& Params)
{
    FastNoise* Noise = nullptr;
    {
        // 线程安全：访问共享缓存
        FScopeLock Lock(&GCriticalSection);
        if (FastNoise** Found = GNoiseCache.Find(Params.Seed))
        {
            Noise = *Found;
        }
        else
        {
            // 创建新噪声实例（按种子隔离）
            Noise = new FastNoise();
            Noise->SetSeed(Params.Seed);
            Noise->SetFrequency(Params.TerrainScale); // 控制地形粗糙度
            Noise->SetNoiseType(FastNoise::Perlin);   // 可替换为 Simplex 更自然
            GNoiseCache.Add(Params.Seed, Noise);
        }
    }

    // FastNoise 返回 [-1, 1]，映射到 [0, HeightMultiplier]
    const float NoiseValue = Noise->GetNoise(WorldX, WorldY);
    const float Normalized = (NoiseValue + 1.0f) * 0.5f;
    const float HeightFloat = Normalized * Params.HeightMultiplier;

    // 限制在有效高度范围内
    return FMath::Clamp(FMath::RoundToInt(HeightFloat), 0, Params.WorldHeight - 1);
}

void UHeightGenerator::GenerateChunkHeights(
    int32 ChunkX,
    int32 ChunkY,
    const FWorldGenParams& Params,
    TArray<int32>& OutHeights)
{
    // 初始化输出数组（16x16 = 256）
    OutHeights.SetNumZeroed(Params.ChunkSize * Params.ChunkSize);

    // 遍历区块内每个格子
    for (int32 x = 0; x < Params.ChunkSize; x++)
    {
        for (int32 y = 0; y < Params.ChunkSize; y++) // y: 区块内局部 Y 坐标
        {
            // 转换为世界坐标（整数格子中心）
            const float WorldX = static_cast<float>(ChunkX * Params.ChunkSize + x);
            const float WorldY = static_cast<float>(ChunkY * Params.ChunkSize + y);

            // 生成该点高度
            const int32 Height = GenerateHeightAt(WorldX, WorldY, Params);
            // 存储到高度图（行主序：x + y * width）
            OutHeights[x + y * Params.ChunkSize] = Height;
        }
    }
}