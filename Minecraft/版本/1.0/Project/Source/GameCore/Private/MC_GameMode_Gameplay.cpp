#include "MC_GameMode_Gameplay.h"
#include "WorldGenerationSubsystem.h"
#include "ChunkActor.h"        // 用于 StaticClass 和类型检查
#include "WorldGenerationConfig.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(H_LogGameMode);

void AMC_GameMode_Gameplay::BeginPlay()
{
    Super::BeginPlay();

    // 获取世界生成子系统
    if (UWorldGenerationSubsystem* WGSS = GetWorld()->GetSubsystem<UWorldGenerationSubsystem>())
    {
        // 设置区块 Actor 类
        if (ChunkActorClass)
        {
            WGSS->SetChunkActorClass(ChunkActorClass);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("ChunkActorClass not assigned in GameMode!"));
        }

        // 设置世界生成配置（异步加载）
        if (WorldGenConfig.IsValid())
        {
            WGSS->SetWorldConfig(WorldGenConfig); // 内部会异步加载并回调

            // 绑定委托，等待加载完成
            WGSS->OnWorldConfigLoaded.AddDynamic(this, &AMC_GameMode_Gameplay::OnWorldConfigReady);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("WorldGenConfig not assigned in GameMode!"));
        }
    }
}

void AMC_GameMode_Gameplay::OnWorldConfigReady()
{
    // 移除委托避免重复触发（安全做法）
    if (UWorldGenerationSubsystem* WGSS = GetWorld()->GetSubsystem<UWorldGenerationSubsystem>())
    {
        WGSS->OnWorldConfigLoaded.RemoveDynamic(this, &AMC_GameMode_Gameplay::OnWorldConfigReady);

        UE_LOG(H_LogGameMode, Log, TEXT("World config ready. Starting terrain generation..."));

        // 获取玩家位置（若游戏刚开始，可能没有 Pawn，用原点兜底）
        FVector PlayerLocation = FVector::ZeroVector;
        if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
        {
            if (APawn* Pawn = PC->GetPawn())
            {
                PlayerLocation = Pawn->GetActorLocation();
            }
        }

        //  触发世界生成！半径=2 表示生成 5x5 = 25 个区块
        WGSS->GenerateWorldAroundPlayer(PlayerLocation, 2);
    }
}
