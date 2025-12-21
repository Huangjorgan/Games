

#pragma once

#include "CoreMinimal.h"

/**
 * 路径管理类
 * 设置存档的具体目录结构
 */
class VOXELPERSISTENCE_API FVoxelPersistencePaths
{
public:
    static FString GetWorldSaveDir(const FString& WorldName);
    static FString GetMetaFilePath(const FString& WorldName);
    static FString GetChunkDir(const FString& WorldName);
    static FString GetChunkFilePath(const FString& WorldName, const FIntPoint& ChunkPos);
};
