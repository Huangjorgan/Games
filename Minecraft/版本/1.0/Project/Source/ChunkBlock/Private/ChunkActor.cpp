#include "ChunkActor.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"  // HISMC批量渲染核心组件
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY(H_LogChunkBlock);
//  构造函数   —— ChunkActor生成时最先执行，用于初始化组件与基础状态
AChunkActor::AChunkActor()
{
	// 禁止Tick — Chunk不会实时逻辑更新，除非你未来做流体、草、光照变化
	PrimaryActorTick.bCanEverTick = false;

	//---------------------- 创建根节点 ----------------------
	// 游戏中所有组件必须存在一个根组件作为坐标参考
	// 场景中Chunk的位置=Root的位置
	// 强制覆盖 RootComponent（关键！必须放最上面！）
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	Root->SetMobility(EComponentMobility::Static);
	RootComponent = Root;

	//---------------------- 创建HISM容器 ----------------------
	// HISMC 允许在 GPU 中批量渲染大量相同网格的实例（方块非常多时性能极高）
	HISMC = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("ChunkHISMC"));
	HISMC->SetupAttachment(RootComponent);         // 绑定到Chunk根节点

	HISMC->SetCanEverAffectNavigation(false);      // 不参与导航（提高性能）
	HISMC->bDisableCollision = true;               // 关闭碰撞，未来你若做方块物理需改
	HISMC->SetCastShadow(false);                   // 关闭阴影，减少开销（未来可按需开启）
	HISMC->SetMobility(EComponentMobility::Static);// Static=顶级优化，Chunk不移动时最合理
}


// BeginPlay — 游戏运行(Possess/Play)时调用，通常用于第二阶段初始化
void AChunkActor::BeginPlay()
{
	Super::BeginPlay();

	// 如果未在蓝图设置，则采用默认Mesh与材质
	// 注意：如果没有Mesh则Chunk永远不会显示
	// 当AtlasMaterial中使用 PerInstanceCustomData[0] 读取BlockID进行贴图切换。
	if (DefaultMesh)
		HISMC->SetStaticMesh(DefaultMesh);

	if (AtlasMaterial)
		HISMC->SetMaterial(0, AtlasMaterial);

}



// InitializeChunk —— 初始化区块尺寸 与 Block数组内存
// 本质：创建一个 [SizeX * SizeY * SizeZ] 大小的方块世界
void AChunkActor::InitializeChunk(int32 InSizeX, int32 InSizeY, int32 InSizeZ, float InBlockSize)
{
	//---------------------------------- 基础尺寸参数 -----------------------------------
	SizeX = FMath::Max(1, InSizeX);
	SizeY = FMath::Max(1, InSizeY);
	SizeZ = FMath::Max(1, InSizeZ);
	BlockSize = InBlockSize;              // UE单位=厘米，128=1.28m

	//---------------------------------- 分配方块数组 -----------------------------------
	Blocks.SetNumZeroed(SizeX * SizeY * SizeZ); // 全部初始化为空气方块ID=0

	//---------------------------------- 清空历史渲染实例 --------------------------------
	if (HISMC)
		HISMC->ClearInstances();


	//---------------------------------- 初始化实例映射表 --------------------------------
	InstanceIndices.Init(-1, SizeX * SizeY * SizeZ);
	InstanceToCell.Reset();

	//---------------------------------- bInstancesDirty=未同步HISM状态 -------------------
	bInstancesDirty = false;

	//---------------------------------- 在这里调用 UpdateInstances() 才能显示渲染 ---
	UpdateInstances();
	// 因为刚填了Blocks但未生成实例，因此不会看见地面
}

// SetBlock —— 设置单个方块（可实时放置 or 挖掘）
bool AChunkActor::SetBlock(int32 X, int32 Y, int32 Z, int32 BlockID, bool bUpdateMesh)
{
	//---------------------------------- 越界检查 -----------------------------------
	if (X < 0 || X >= SizeX || Y < 0 || Y >= SizeY || Z < 0 || Z >= SizeZ)
		return false;

	const int32 Index = ToIndex(X, Y, Z);
	const int32 OldID = Blocks[Index];
	Blocks[Index] = BlockID;

	//---------------------------------- 若无需立即更新渲染，则仅标记脏区 -----------------
	if (!bUpdateMesh)
	{
		bInstancesDirty = true;
		return true;
	}

	//---------------------------------- 以下进入实时增量更新模式 --------------------------
	// 优点：修改单一方块时不必重建整个chunk（极大提升效率）

	if (!HISMC)
	{
		if (bUpdateMesh) UpdateInstances(); // 防御写法
		return true;
	}

	// 空->方块 = AddInstance
	if (OldID == 0 && BlockID != 0)
	{
		//!! 这里调试时需要注意vector是以世界中心为原点还是以chunk原点为原点
		FVector Pos = FVector(X * BlockSize, Y * BlockSize, Z * BlockSize);
		FTransform TF(FRotator::ZeroRotator, Pos, FVector(BlockSize / 128.f)); // 非统一缩放注意点!!!
		AddBlockInstanceAt(Index, BlockID, TF);
	}

	// 方块->空气 = RemoveInstance
	else if (OldID != 0 && BlockID == 0)
		RemoveBlockInstanceAt(Index);

	// 方块ID改变 = Remove再Add（可未来优化为CustomData更新直接变材质）
	else if (OldID != 0 && BlockID != 0 && OldID != BlockID)
	{
		//!! 这里调试时需要注意vector是以世界中心为原点还是以chunk原点为原点
		FVector Pos = FVector(X * BlockSize, Y * BlockSize, Z * BlockSize);
		FTransform TF(FRotator::ZeroRotator, Pos, FVector(BlockSize / 128.f));
		RemoveBlockInstanceAt(Index);
		AddBlockInstanceAt(Index, BlockID, TF);
	}

	return true;
}

// GetBlock —— 返回世界中某格子的方块ID
int32 AChunkActor::GetBlock(int32 X, int32 Y, int32 Z) const
{
	if (X < 0 || X >= SizeX || Y < 0 || Y >= SizeY || Z < 0 || Z >= SizeZ)
		return 0;
	return Blocks[ToIndex(X, Y, Z)];
}

// 建立并刷新所有HISM实例（全量更新，支持数千方块）
void AChunkActor::UpdateInstances()
{
	// 若Chunk不在世界或渲染无效则退出
	if (!HISMC || !GetWorld())
		return;

	HISMC->ClearInstances(); // 清除历史实例 → 将重新创建全部方块

	// 运行时兜底安全设置Mesh与材质
	if (!HISMC->GetStaticMesh() && DefaultMesh)
		HISMC->SetStaticMesh(DefaultMesh);
	if (AtlasMaterial)
		HISMC->SetMaterial(0, AtlasMaterial);

	UE_LOG(H_LogChunkBlock, Log, TEXT("AChunkActor::UpdateInstances() - Rebuilding chunk instances..."));
	TArray<FTransform> InstanceTransforms; // 储存所有方块的local transform
	TArray<float> CustomData;              // 每实例携带的自定义数据 buffer

	InstanceTransforms.Reserve(SizeX * SizeY * SizeZ);
	CustomData.Reserve(SizeX * SizeY * SizeZ * NumCustomDataFloatsPerInstance);

	//=================== 遍历所有Block，收集可见方块 ===================
	for (int Z = 0; Z < SizeZ; Z++)
		for (int Y = 0; Y < SizeY; Y++)
			for (int X = 0; X < SizeX; X++)
			{
				int32 Index = ToIndex(X, Y, Z);
				int32 ID = Blocks[Index];

				if (ID == 0) continue; // 空气方块不渲染

				FVector Pos;
				Pos.X = X * BlockSize;
				Pos.Y = Y * BlockSize;
				Pos.Z = Z * BlockSize;

				// 实例transform —— 位置 + 坐标 + 缩放
				FTransform TF(FRotator::ZeroRotator, Pos, FVector(BlockSize / 128.0f));
				InstanceTransforms.Add(TF);

				// 自定义材质数据(0号float存BlockID,材质可按ID解析atlas)
				CustomData.Add((float)ID);
				for (int e = 1; e < NumCustomDataFloatsPerInstance; e++)
					CustomData.Add(0.f);
			}

	//=================== 批量渲染构建 ===================
	if (InstanceTransforms.Num() > 0)
	{
		InstanceIndices.Init(-1, Blocks.Num()); // cell->instance映射初始化空
		InstanceToCell.Reset();

		HISMC->NumCustomDataFloats = NumCustomDataFloatsPerInstance;
		HISMC->PerInstanceSMCustomData.SetNumZeroed(InstanceTransforms.Num() * NumCustomDataFloatsPerInstance);

		// 逐个AddInstance并建立 Instance ↔ Block 映射
		for (int i = 0; i < InstanceTransforms.Num(); i++)
		{
			int32 InstID = HISMC->AddInstance(InstanceTransforms[i]);	// 返回实例序号

			InstanceToCell.Add(i);	// !!! 当前写法并未存储CellIndex（未来可优化
			// 使用CellIndices数组更稳定，支持移除与更新

// CustomData 直接赋值用于材质获取BlockID
			int Offset = InstID * NumCustomDataFloatsPerInstance;
			HISMC->PerInstanceSMCustomData[Offset] = CustomData[i * NumCustomDataFloatsPerInstance];
		}

		HISMC->BuildTreeIfOutdated(true, false);	// HISM构建LOD/层级加速
		bInstancesDirty = false;
	}
}

// AddBlockInstanceAt —— 单格新增方块实例
bool AChunkActor::AddBlockInstanceAt(int32 CellIndex, int32 BlockID, const FTransform& Transform)
{
	if (!HISMC) return false;
	if (!ensure(NumCustomDataFloatsPerInstance >= 1)) return false;

	// 如果格子已经存在实例——直接改材质，而不重新AddInstance（性能更好）
	// 这个情况一般出现在方块替换时
	if (InstanceIndices.IsValidIndex(CellIndex) && InstanceIndices[CellIndex] != -1)
	{
		int32 InstID = InstanceIndices[CellIndex];
		int Offset = InstID * NumCustomDataFloatsPerInstance;

		if (HISMC->PerInstanceSMCustomData.IsValidIndex(Offset))
		{
			HISMC->PerInstanceSMCustomData[Offset] = (float)BlockID;
			HISMC->MarkRenderStateDirty(); // 重新触发材质更新
		}
		return true;
	}

	// 添加新实例
	HISMC->NumCustomDataFloats = NumCustomDataFloatsPerInstance;
	int32 NewInstID = HISMC->AddInstance(Transform);

	int Size = (NewInstID + 1) * NumCustomDataFloatsPerInstance;
	if (HISMC->PerInstanceSMCustomData.Num() < Size)
		HISMC->PerInstanceSMCustomData.SetNumZeroed(Size);

	// 写入材质ID
	int Offset = NewInstID * NumCustomDataFloatsPerInstance;
	HISMC->PerInstanceSMCustomData[Offset] = (float)BlockID;

	// 建立双向映射（非常关键————移除时依赖！）
	InstanceIndices[CellIndex] = NewInstID;
	if (InstanceToCell.Num() <= NewInstID)
		InstanceToCell.SetNumZeroed(NewInstID + 1);
	InstanceToCell[NewInstID] = CellIndex;

	HISMC->MarkRenderStateDirty();
	return true;
}

// RemoveBlockInstanceAt —— 删除格子方块并保持HISM索引连续
bool AChunkActor::RemoveBlockInstanceAt(int32 CellIndex)
{
	if (!HISMC || !InstanceIndices.IsValidIndex(CellIndex))return false;

	int RemoveID = InstanceIndices[CellIndex];
	if (RemoveID < 0) return false;	// 格子无方块直接返回

	int LastID = HISMC->GetInstanceCount() - 1;
	int Stride = NumCustomDataFloatsPerInstance;

	// 若删除的不是最后一个，则交换数据保持连续
	if (RemoveID != LastID)
	{
		int LastCell = InstanceToCell.IsValidIndex(LastID) ? InstanceToCell[LastID] : -1;
		if (LastCell != -1) {
			InstanceIndices[LastCell] = RemoveID; // 更新cell映射至移位后索引
			InstanceToCell[RemoveID] = LastCell;
		}

		// 交换材质数据 → 保证ID连续不碎裂
		if (HISMC->PerInstanceSMCustomData.Num() >= (LastID + 1) * Stride)
		{
			int src = LastID * Stride, dst = RemoveID * Stride;
			for (int i = 0; i < Stride; i++)
				HISMC->PerInstanceSMCustomData[dst + i] = HISMC->PerInstanceSMCustomData[src + i];
		}
	}

	// 调用引擎内部Remove → 完成渲染结构更新
	bool ok = HISMC->RemoveInstance(RemoveID);
	if (!ok) return false;

	// 删除custom数据尾部
	if (HISMC->PerInstanceSMCustomData.Num() >= Stride)
		HISMC->PerInstanceSMCustomData.RemoveAt(LastID * Stride, Stride, true);

	InstanceIndices[CellIndex] = -1;
	InstanceToCell.RemoveAt(LastID);
	HISMC->MarkRenderStateDirty();
	return true;
}

// IChunkInterface Implementation —— 供 WorldGen 模块调用
void AChunkActor::SetChunkData(const TArray<int32>& BlockData)
{
	//=================== 调试日志 ===================
	UE_LOG(H_LogChunkBlock, Log, TEXT("AChunkActor::SetChunkData: Coords=(%d,%d,%d), DataSize=%d"),
		ChunkCoordinates.X, ChunkCoordinates.Y, ChunkCoordinates.Z, BlockData.Num());

	// 安全检查：数据长度必须匹配当前区块体积
	if (BlockData.Num() != SizeX * SizeY * SizeZ)
	{
		UE_LOG(H_LogChunkBlock, Warning, TEXT("AChunkActor::SetChunkData: Data size mismatch! Expected %d, got %d"),
			SizeX * SizeY * SizeZ, BlockData.Num());
		return;
	}

	Blocks = BlockData;
	bInstancesDirty = true; // 标记为脏，但不立即更新（由 RefreshRendering 触发）
}

void AChunkActor::SetChunkCoordinates(FIntVector Coords)
{
	ChunkCoordinates = Coords;
}

FIntVector AChunkActor::GetChunkCoordinates() const
{
	return ChunkCoordinates;
}

void AChunkActor::RefreshRendering()
{
	//=================== 调试日志 ===================
	UE_LOG(H_LogChunkBlock, Log, TEXT("Refreshing rendering for chunk (%d,%d)"),
		ChunkCoordinates.X, ChunkCoordinates.Y);

	// 如果已标记为脏，或尚未生成实例，则强制重建
	if (bInstancesDirty || !HISMC || HISMC->GetInstanceCount() == 0)
	{
		UpdateInstances(); // 调用你已有的全量更新函数
	}
}

void AChunkActor::SetChunkVoxelData(const TArray<int32>& InVoxelData)
{
	// 安全检查：长度必须匹配
	if (InVoxelData.Num() != SizeX * SizeY * SizeZ)
	{
		UE_LOG(H_LogChunkBlock, Error, TEXT("SetChunkVoxelData: Data size mismatch! Expected %d, got %d"),
			SizeX * SizeY * SizeZ, InVoxelData.Num());
		return;
	}

	Blocks = InVoxelData;
	bInstancesDirty = true; // 标记为脏，下次 RefreshRendering 会更新渲染
}

const TArray<int32>& AChunkActor::GetChunkVoxelData() const
{
	return Blocks;
}