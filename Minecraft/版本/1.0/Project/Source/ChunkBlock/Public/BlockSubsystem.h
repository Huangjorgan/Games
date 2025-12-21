
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "BlockSubsystem.generated.h"

class UBlockType;
/**
 * 世界范围的方块注册表（WorldSubsystem）
 * 在运行时注册与查找方块定义
 */
UCLASS()
class CHUNKBLOCK_API UBlockSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// Subsystem 生命周期
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// 注册方块数据资产
	void RegisterBlock(UBlockType* Block);

	// 按 ID / 名称查找
	UBlockType* FindBlockByID(int32 BlockID) const;
	UBlockType* FindBlockByName(FName Name) const;

private:
	// 映射：ID -> BlockType
	UPROPERTY()
	TMap<int32, TWeakObjectPtr<UBlockType>> BlocksByID;

	// 映射：Name -> BlockType
	UPROPERTY()
	TMap<FName, TWeakObjectPtr<UBlockType>> BlocksByName;
};
