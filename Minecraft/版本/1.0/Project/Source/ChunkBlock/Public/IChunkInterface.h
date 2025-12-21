
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IChunkInterface.generated.h"

/**
 * IChunkInterface - 区块操作的抽象接口
 * 由 ChunkBlock 模块提供实现，WorldGen 模块仅依赖此接口
 */
UINTERFACE(MinimalAPI)
class UChunkInterface : public UInterface
{
    GENERATED_BODY()
};

class IChunkInterface
{
    GENERATED_BODY()

public:
    /**
     * 设置整个区块的方块数据（一维数组）
     * 数组长度 = SizeX * WorldHeight * SizeZ
     */
    virtual void SetChunkData(const TArray<int32>& BlockData) = 0;

    /**
     * 设置该区块在世界中的逻辑坐标（以区块为单位）
     * 例如：(0,0), (1,-2) 等
     */
    virtual void SetChunkCoordinates(FIntVector Coords) = 0;

    /**
     * 获取当前区块的逻辑坐标
     */
    virtual FIntVector GetChunkCoordinates() const = 0;

    /**
     * 刷新渲染（重建 HISMC 实例）
     */
    virtual void RefreshRendering() = 0;

	//区块体素数据相关接口
    virtual void SetChunkVoxelData(const TArray<int32>& InVoxelData) = 0;
    virtual const TArray<int32>& GetChunkVoxelData() const = 0;
};