#include "ChunkGenerationManager.h"
#include "WorldGenerationConfig.h"
#include "HeightGenerator.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "LogWorldGeneration.h"

void UChunkGenerationManager::Initialize(UWorldGenerationConfig* Config)
{
    CurrentConfig = Config;
}

AActor* UChunkGenerationManager::RequestChunk(int32 ChunkX, int32 ChunkY, UWorld* World)
{
	// 记录请求日志
    UE_LOG(H_LogWorldGeneration, Log, TEXT("Requesting chunk at (%d, %d)"), ChunkX, ChunkY);

    if (!CurrentConfig || !World)
        return nullptr;

    // 构建区块唯一键（X-Y 平面坐标）
    const FIntPoint ChunkKey(ChunkX, ChunkY);

    // 检查是否已加载
    if (TWeakObjectPtr<AActor>* FoundPtr = LoadedChunks.Find(ChunkKey))
    {
        if (AActor* ExistingChunk = FoundPtr->Get())
        {
            return ExistingChunk; // 返回现有区块
        }
        // 弱引用失效（Actor 已被销毁），继续创建新实例
    }

    // 生成新区块
    AActor* NewChunk = SpawnChunkActor(ChunkX, ChunkY, World);
    if (!NewChunk)
        return nullptr;

    // 缓存并生成数据
    LoadedChunks.Add(ChunkKey, NewChunk);
    GenerateChunkData(NewChunk, ChunkX, ChunkY);
    return NewChunk;
}

AActor* UChunkGenerationManager::SpawnChunkActor(int32 ChunkX, int32 ChunkY, UWorld* World)
{
    if (!ChunkActorClass)
    {
        UE_LOG(H_LogWorldGeneration, Error, TEXT("ChunkActorClass not set!"));
        return nullptr;
    }
    
    // 每个区块物理尺寸：16 格 × 128 cm/格 = 2048 cm
    const float ChunkSizeCM = 16.0f * 128.0f;
    // 在 X-Y 平面放置区块，Z=0 表示地面高度（后续由体素数据决定实际地形高度）
    const FVector Location(ChunkX * ChunkSizeCM, ChunkY * ChunkSizeCM, 0.0f);

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    SpawnParams.Owner = nullptr;

    AActor* Chunk = World->SpawnActor<AActor>(ChunkActorClass, Location, FRotator::ZeroRotator, SpawnParams);   
    if (!Chunk)
        return nullptr;
    
	//============== 区块边界调试显示 ==============
#if !(UE_BUILD_SHIPPING)
    DrawDebugBox(World, Location + FVector(1024, 1024, 512), FVector(1024, 1024, 512), FColor::Green, false, 1000.f);
#endif

    // 通知区块 Actor 设置其逻辑坐标（Z=0 表示无垂直分块）
    if (IChunkInterface* CI = Cast<IChunkInterface>(Chunk))
    {
        CI->SetChunkCoordinates(FIntVector(ChunkX, ChunkY, 0));
    }
    else
    {
        UE_LOG(H_LogWorldGeneration, Error, TEXT("Spawned actor does not implement IChunkInterface!"));
        Chunk->Destroy();
        return nullptr;
    }

    return Chunk;
}

void UChunkGenerationManager::GenerateChunkData(AActor* Chunk, int32 ChunkX, int32 ChunkY)
{
    if (!CurrentConfig || !Chunk)
        return;

    const FWorldGenParams& Params = CurrentConfig->Params;

    // Step 1: 生成 16x16 地表高度图（每个 (x,y) 对应一个地表 Z 值）
    TArray<int32> SurfaceHeights;
    UHeightGenerator::GenerateChunkHeights(ChunkX, ChunkY, Params, SurfaceHeights); // 返回 256 个值，按 x + y*16 存储

    // Step 2: 分配三维体素数据 [X=16][Y=16][Z=WorldHeight]
    const int32 TotalBlocks = 16 * 16 * Params.WorldHeight; 
    TArray<int32> Blocks;
    Blocks.SetNumZeroed(TotalBlocks); // 0 = 空气

    // Step 3: 填充体素
    for (int32 x = 0; x < 16; x++)          // X: 水平方向
    {
        for (int32 y = 0; y < 16; y++)      // Y: 水平方向
        {
            // 获取 (x,y) 处的地表高度（Z 坐标）
            const int32 SurfaceZ = SurfaceHeights[x + y * 16]; // HeightGenerator 输出按 x + y*16 存储

            // 从底部 (Z=0) 填充到地表 (Z=SurfaceZ)
            for (int32 z = 0; z <= SurfaceZ && z < Params.WorldHeight; z++)
            {
                //世界Z方向方块分配规则
                const int32 BlockID = (z == 0) ? 2 :
                    (z == SurfaceZ) ? 1 :
                    (z >= SurfaceZ - 2 && z < SurfaceZ) ? 7 :
                    3;
                const int32 Index = x + y * 16 + z * (16 * 16); // ← 关键：z * (SizeX * SizeY)
                if (Index >= 0 && Index < Blocks.Num())
                {
                    Blocks[Index] = BlockID;
                }
            }
        }
    }

    // 传递给 Chunk Actor
    if (IChunkInterface* CI = Cast<IChunkInterface>(Chunk))
    {
        CI->SetChunkData(Blocks);
        CI->RefreshRendering();
    }
}

void UChunkGenerationManager::UnloadDistantChunks(const FIntPoint& PlayerChunkPos, int32 RenderDistance)
{
    if (!CurrentConfig || RenderDistance <= 0)
        return;

    // 计算最大允许距离平方（+1 防止边缘闪烁）
    const int32 MaxDistSq = (RenderDistance + 1) * (RenderDistance + 1);
    TArray<FIntPoint> ChunksToRemove;

    // 遍历所有已加载区块
    for (const auto& Pair : LoadedChunks)
    {
        const FIntPoint& ChunkKey = Pair.Key;
        TWeakObjectPtr<AActor>& ChunkPtr = const_cast<TWeakObjectPtr<AActor>&>(Pair.Value);

        // 清理已销毁的弱引用
        if (!ChunkPtr.IsValid())
        {
            ChunksToRemove.Add(ChunkKey);
            continue;
        }

        // 检查是否超出可视范围
        if (GetChunkDistanceSq(ChunkKey, PlayerChunkPos) > MaxDistSq)
        {
            if (AActor* ChunkActor = ChunkPtr.Get())
            {
                ChunkActor->Destroy();
                UE_LOG(H_LogWorldGeneration, Verbose, TEXT("Unloaded chunk at (%d, %d)"), ChunkKey.X, ChunkKey.Y);
            }
            ChunksToRemove.Add(ChunkKey);
        }
    }

    // 从缓存中移除已卸载的区块
    for (const FIntPoint& Key : ChunksToRemove)
    {
        LoadedChunks.Remove(Key);
    }
}