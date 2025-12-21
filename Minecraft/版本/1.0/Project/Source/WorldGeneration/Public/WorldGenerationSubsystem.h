#pragma once
#include "Subsystems/WorldSubsystem.h"
#include "Engine/AssetManager.h"
#include "WorldGenerationSubsystem.generated.h"

class UChunkGenerationManager;
class UWorldGenerationConfig;

//  新增：声明动态多播委托（支持蓝图绑定）
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWorldConfigLoadedDelegate);

/**
 * @brief 世界生成子系统
 *
 * 作为全局子系统，协调区块加载、卸载与渲染。
 * 绑定到当前关卡 UWorldSubsystem，在关卡开始/结束时自动创建/销毁。
 */
UCLASS()
class WORLDGENERATION_API UWorldGenerationSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** 子系统初始化（引擎自动调用） */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/**
	 * @brief 异步设置世界生成配置
	 * @param Config 配置资源软引用，支持异步加载
	 */
	UFUNCTION(BlueprintCallable, Category = "WorldGen")
	void SetWorldConfig(TSoftObjectPtr<UWorldGenerationConfig> Config);

	/**
	 * @brief 围绕玩家位置生成/卸载区块
	 *
	 * 通过每帧调用实现动态加载，适用于玩家移动时触发
	 *
	 * @param PlayerLocation 玩家世界坐标（FVector）
	 * @param Radius 渲染半径（单位：区块数量）
	 */
	UFUNCTION(BlueprintCallable, Category = "WorldGen")
	void GenerateWorldAroundPlayer(const FVector& PlayerLocation, int32 Radius);

	/** 设置区块 Actor 类型（由外部指定） */
	UFUNCTION(BlueprintCallable, Category = "WorldGen")
	void SetChunkActorClass(TSubclassOf<AActor> InClass);

	/** 公开 ChunkManager 供调试或扩展 */
	UPROPERTY(BlueprintReadOnly, Category = "WorldGen")
	TObjectPtr<UChunkGenerationManager> ChunkManager;

	//  新增：公开委托，供 GameMode 或蓝图监听
	UPROPERTY(BlueprintAssignable, Category = "WorldGen")
	FOnWorldConfigLoadedDelegate OnWorldConfigLoaded;

private:
	/** 配置资源软引用，避免硬依赖 */
	TSoftObjectPtr<UWorldGenerationConfig> ConfigSoftPtr;

	/** 区块 Actor 类（由外部设置） */
	TSubclassOf<AActor> ChunkActorClass;

	/** 防止重复加载 */
	bool bIsLoadingConfig = false;

	/** 异步加载完成回调 */
	void OnWorldConfigLoadedCallback();
};