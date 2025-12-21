
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IChunkInterface.h" 
#include "ChunkActor.generated.h"
DECLARE_LOG_CATEGORY_EXTERN(H_LogChunkBlock, Log, All);
class UHierarchicalInstancedStaticMeshComponent;
class UMaterialInterface;

/**
 * AChunkActor 责任是维护一个 SizeXSizeYSizeZ 的格子数组（Blocks），
 * 用单个 UHierarchicalInstancedStaticMeshComponent(HISMC) 渲染非空方块。
 * 不同方块材质问题使用Atlas图集＋每实例自定义数据解决。
 * 渲染可通过全量重建或增量增删实例完成，代码两种策略都有体现。
 */

UCLASS()
class CHUNKBLOCK_API AChunkActor : public AActor, public IChunkInterface
{
	GENERATED_BODY()
	
public:	
	AChunkActor();

	// 初始化区块尺寸与方块尺寸（会分配 Blocks 数组）
	UFUNCTION(BlueprintCallable, Category = "Chunk")
	void InitializeChunk(int32 InSizeX, int32 InSizeY, int32 InSizeZ, float InBlockSize = 100.f);

	// 设置/查询方块（BlockID：0=空气）
	UFUNCTION(BlueprintCallable, Category = "Chunk")
	bool SetBlock(int32 X, int32 Y, int32 Z, int32 BlockID, bool bUpdateMesh = true);

	UFUNCTION(BlueprintCallable, Category = "Chunk")
	int32 GetBlock(int32 X, int32 Y, int32 Z) const;

	// 重新生成 HISMC 实例（全量重建）
	UFUNCTION(BlueprintCallable, Category = "Chunk")
	void UpdateInstances();

	// 体素数据接口实现
	virtual void SetChunkVoxelData(const TArray<int32>& InVoxelData) override;
	virtual const TArray<int32>& GetChunkVoxelData() const override;
protected:
	virtual void BeginPlay() override;

	FORCEINLINE int32 ToIndex(int32 X, int32 Y, int32 Z) const { return X + Y * SizeX + Z * SizeX * SizeY; }

	// 区块格子尺寸
	UPROPERTY(EditAnywhere, Category = "Chunk")
	int32 SizeX = 16;

	UPROPERTY(EditAnywhere, Category = "Chunk")
	int32 SizeY = 16;

	UPROPERTY(EditAnywhere, Category = "Chunk")
	int32 SizeZ = 16;

	UPROPERTY(EditAnywhere, Category = "Chunk")
	float BlockSize = 128.0f;

	// 存储 BlockID（1D）
	UPROPERTY()
	TArray<int32> Blocks;

	// 延迟同步标志：当批量修改 Blocks（bUpdateMesh == false）时设为 true，
	// 后续调用 UpdateInstances() 会把渲染实例与 Blocks 同步并清除此标志。
	UPROPERTY()
	bool bInstancesDirty = false;

	// 增量实例管理映射：
	// - 通过方块坐标找到对应 HISMC 实例
	UPROPERTY()
	TArray<int32> InstanceIndices;

	// - 删除实例时找到其原方块位置进行重建映射
	UPROPERTY()
	TArray<int32> InstanceToCell;

	// 增量添加/移除单个实例（内部使用）
	bool AddBlockInstanceAt(int32 CellIndex, int32 BlockID, const FTransform& Transform);
	bool RemoveBlockInstanceAt(int32 CellIndex);

	// 单个 HISMC 用于所有方块实例化渲染
	UPROPERTY(VisibleAnywhere, Category = "Chunk")
	UHierarchicalInstancedStaticMeshComponent* HISMC = nullptr;

	// 用作实例化网格（所有方块共享同一网格）
	UPROPERTY(EditAnywhere, Category = "Chunk|Rendering")
	UStaticMesh* DefaultMesh = nullptr;

	// 使用的精灵图集材质（材质需读取 PerInstanceCustomData[0]）
	UPROPERTY(EditAnywhere, Category = "Chunk|Rendering")
	UMaterialInterface* AtlasMaterial = nullptr;

	// 每实例自定义数据浮点数量（此处使用 2，用于传 atlas 索引和DestroyStage）
	UPROPERTY(EditAnywhere, Category = "Chunk|Rendering")
	int32 NumCustomDataFloatsPerInstance = 2;

	// ———————— IChunkInterface 实现 ————————
	virtual void SetChunkData(const TArray<int32>& BlockData) override;
	virtual void SetChunkCoordinates(FIntVector Coords) override;
	virtual FIntVector GetChunkCoordinates() const override;
	virtual void RefreshRendering() override;

	FIntVector ChunkCoordinates = FIntVector::ZeroValue;

};
