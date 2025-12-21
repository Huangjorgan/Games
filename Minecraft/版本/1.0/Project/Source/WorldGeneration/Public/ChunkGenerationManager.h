#pragma once

#include "CoreMinimal.h"
#include "IChunkInterface.h"
#include "UObject/Object.h"
#include "ChunkGenerationManager.generated.h"

class UWorldGenerationConfig;

/**
 * @brief 区块生成管理器
 *
 * 负责按需生成、缓存和卸载地形区块（Chunk）。
 *
 *  坐标系说明（Unreal 默认）：
 * - X 轴：水平向右（Right）
 * - Y 轴：水平向前（Forward）
 * - Z 轴：垂直向上（Up） ← 高度方向
 *
 * 因此，地形区块在 **X-Y 平面** 上铺展，每个区块内部沿 **Z 轴** 构建高度。
 */
UCLASS()
class WORLDGENERATION_API UChunkGenerationManager : public UObject
{
    GENERATED_BODY()

public:
    /**
     * @brief 初始化生成器配置
     * @param Config 世界生成参数（种子、缩放、高度等）
     */
    void Initialize(UWorldGenerationConfig* Config);

    /**
     * @brief 请求指定坐标的区块
     *
     * 若区块已加载，则返回现有 Actor；否则生成新 Actor。
     *
     * @param ChunkX 区块在 X 轴的索引（整数，如 -1, 0, 1...）
     * @param ChunkY 区块在 Y 轴的索引（整数）
     * @param World 用于生成 Actor 的世界对象
     * @return 生成的或已存在的区块 Actor，失败返回 nullptr
     */
    AActor* RequestChunk(int32 ChunkX, int32 ChunkY, UWorld* World);

    /**
     * @brief 卸载远离玩家的区块以节省内存
     *
     * 遍历已加载区块，销毁超出渲染距离的区块 Actor，并从缓存中移除。
     *
     * @param PlayerChunkPos 玩家当前所在的区块坐标 (X, Y)
     * @param RenderDistance 渲染半径（单位：区块数）
     */
    void UnloadDistantChunks(const FIntPoint& PlayerChunkPos, int32 RenderDistance);

    /**
     * @brief 指定用于生成区块的 Actor 类
     *
     * 必须继承自 AActor 并实现 IChunkInterface 接口。
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
    TSubclassOf<AActor> ChunkActorClass;

    /** 当前生效的世界生成配置 */
    TObjectPtr<UWorldGenerationConfig> CurrentConfig;

private:
    /**
     * @brief 已加载区块的缓存映射
     *
     * 键：区块逻辑坐标 (ChunkX, ChunkY)
     * 值：弱引用指向区块 Actor（避免阻止 GC）
     */
    TMap<FIntPoint, TWeakObjectPtr<AActor>> LoadedChunks;
 
    /**
     * @brief 实际生成区块 Actor 并设置其世界位置
     * @return 新生成的 Actor，失败返回 nullptr
     */
    AActor* SpawnChunkActor(int32 ChunkX, int32 ChunkY, UWorld* World);

    /**
     * @brief 为区块生成体素数据（方块 ID 数组）
     *
     * 调用 HeightGenerator 生成地表高度，然后填充土/草方块。
     */
    void GenerateChunkData(AActor* Chunk, int32 ChunkX, int32 ChunkY);

    /**
     * @brief 计算两个区块之间的欧氏距离平方（避免开方运算）
     * @return 距离的平方值
     */
    FORCEINLINE int32 GetChunkDistanceSq(const FIntPoint& A, const FIntPoint& B) const
    {
        int32 dx = A.X - B.X;
        int32 dy = A.Y - B.Y;
        return dx * dx + dy * dy;
    }
}; 