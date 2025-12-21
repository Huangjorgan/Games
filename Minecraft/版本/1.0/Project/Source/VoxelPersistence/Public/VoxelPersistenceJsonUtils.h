

#pragma once

#include "CoreMinimal.h"
#include "VoxelPersistenceTypes.h"
#include "Dom/JsonObject.h"
/**
 * JSON 序列化和反序列化工具类
 */
class VOXELPERSISTENCE_API FVoxelPersistenceJsonUtils
{
public:
    // FVoxelWorldMeta
    static TSharedPtr<FJsonObject> ToJson(const FVoxelWorldMeta& Meta);
    static bool FromJson(const TSharedPtr<FJsonObject>& Json, FVoxelWorldMeta& OutMeta);

    // FVoxelChunkData
    static TSharedPtr<FJsonObject> ToJson(const FVoxelChunkData& Chunk);
    static bool FromJson(const TSharedPtr<FJsonObject>& Json, FVoxelChunkData& OutChunk);

    // 文件 I/O（同步）
    static bool SaveJsonToFile(const FString& FilePath, const TSharedPtr<FJsonObject>& Json);
    static bool LoadJsonFromFile(const FString& FilePath, TSharedPtr<FJsonObject>& OutJson);

private:
    static TSharedPtr<FJsonValue> VectorToJson(const FVector& V);
    static FVector JsonToVector(const TArray<TSharedPtr<FJsonValue>>& Array);
    static TSharedPtr<FJsonValue> IntPointToJson(const FIntPoint& P);
    static FIntPoint JsonToIntPoint(const TSharedPtr<FJsonObject>& Obj);
};
