
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MC_GameMode_Gameplay.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(H_LogGameMode, Log, All);

class AChunkActor;
class UWorldGenerationConfig;
/**
 * MC_GameMode_Gameplay
 * 游戏主模式类，控制游戏的主要玩法逻辑
 */
UCLASS()
class GAMECORE_API AMC_GameMode_Gameplay : public AGameMode
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;

	// 监听 WorldGenerationConfig Loaded
	UFUNCTION()
	void OnWorldConfigReady();


	// 区块 Actor 类（需实现 IChunkInterface）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "World Generation")
	TSubclassOf<AActor> ChunkActorClass;

	// 世界生成配置（软引用，支持异步加载）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Generation")
	TSoftObjectPtr<UWorldGenerationConfig> WorldGenConfig;

};
