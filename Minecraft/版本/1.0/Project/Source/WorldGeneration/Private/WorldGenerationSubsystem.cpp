#include "WorldGenerationSubsystem.h"
#include "ChunkGenerationManager.h"
#include "WorldGenerationConfig.h"
#include "Engine/World.h"
#include "LogWorldGeneration.h"

void UWorldGenerationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    // 创建区块管理器实例
    ChunkManager = NewObject<UChunkGenerationManager>(this);
}

void UWorldGenerationSubsystem::SetChunkActorClass(TSubclassOf<AActor> InClass)
{
    ChunkActorClass = InClass;
    if (ChunkManager)
    {
        ChunkManager->ChunkActorClass = InClass;
    }
}

void UWorldGenerationSubsystem::SetWorldConfig(TSoftObjectPtr<UWorldGenerationConfig> Config)
{
    if (bIsLoadingConfig || Config == ConfigSoftPtr)
        return;

    ConfigSoftPtr = Config;
    if (ConfigSoftPtr.IsValid())
    {
        bIsLoadingConfig = true;
        // 使用 Asset Manager 异步加载配置资源
        FStreamableManager& StreamableManager = UAssetManager::Get().GetStreamableManager();
        StreamableManager.RequestAsyncLoad(
            ConfigSoftPtr.ToSoftObjectPath(),
            FStreamableDelegate::CreateUObject(this, &UWorldGenerationSubsystem::OnWorldConfigLoadedCallback),
            FStreamableManager::AsyncLoadHighPriority
        );
    }
}

void UWorldGenerationSubsystem::OnWorldConfigLoadedCallback()
{
    bIsLoadingConfig = false;
    if (UWorldGenerationConfig* LoadedConfig = ConfigSoftPtr.Get())
    {
        if (ChunkManager)
        {
            ChunkManager->Initialize(LoadedConfig);
            UE_LOG(H_LogWorldGeneration, Log, TEXT("World config loaded asynchronously: %s"), *LoadedConfig->GetName());
        
            OnWorldConfigLoaded.Broadcast();
        }
    }
    else
    {
        UE_LOG(H_LogWorldGeneration, Warning, TEXT("Failed to load WorldGenerationConfig asynchronously!"));
    }
}

void UWorldGenerationSubsystem::GenerateWorldAroundPlayer(const FVector& PlayerLocation, int32 Radius)
{
    if (!GetWorld() || !ChunkManager || !ChunkManager->CurrentConfig || !ChunkManager->ChunkActorClass)
    {
        UE_LOG(H_LogWorldGeneration, Warning, TEXT("Cannot generate world: missing config or ChunkActorClass"));
        return;
    }

    // 计算玩家所在区块坐标
    const float ChunkSizeCM = 16 * 128.0f; // 每区块 2048 cm
    const int32 PlayerChunkX = FMath::FloorToInt(PlayerLocation.X / ChunkSizeCM);
    const int32 PlayerChunkY = FMath::FloorToInt(PlayerLocation.Y / ChunkSizeCM); 

    const FIntPoint PlayerChunkPos(PlayerChunkX, PlayerChunkY);

    // 卸载远处区块
    ChunkManager->UnloadDistantChunks(PlayerChunkPos, Radius);

    // 生成周围区块（方形区域）
    for (int32 dx = -Radius; dx <= Radius; dx++)
    {
        for (int32 dy = -Radius; dy <= Radius; dy++)
        {
            const int32 TargetChunkX = PlayerChunkX + dx;
            const int32 TargetChunkY = PlayerChunkY + dy;
            ChunkManager->RequestChunk(TargetChunkX, TargetChunkY, GetWorld());
        }
    }
}