
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BlockType.generated.h"

/**
 * 方块数据资产（DataAsset）
 * 包含：方块名、方块ID、耐久度、是否可被摧毁、用于渲染的静态网格
 */
UCLASS()
class CHUNKBLOCK_API UBlockType : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	// 用于显示与查询的名字（可与 Asset Name 不同）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Block")
	FName BlockName;

	// 唯一整数 ID（0 保留为空/空气）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Block")
	int32 BlockID = 0;

	// 耐久度（被破坏需要的时间）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Block")
	float Durability = 100.0f;

	// 是否可以被摧毁（false 表示不可破坏）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Block")
	bool bDestructible = true;

	// 用于区块渲染的静态网格（单个方块尺寸假定为 BlockSize）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Block")
	UStaticMesh* Mesh = nullptr;
};
