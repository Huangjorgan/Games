// VoxelPersistenceJsonUtils.cpp
#include "VoxelPersistenceJsonUtils.h"
#include "VoxelPersistencePaths.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonReader.h"
#include "LogVoxelPersistence.h"

// --- 工具函数：Vector <-> JSON ---
TSharedPtr<FJsonValue> FVoxelPersistenceJsonUtils::VectorToJson(const FVector& V)
{
    return MakeShareable(new FJsonValueArray({
        MakeShareable(new FJsonValueNumber(V.X)),
        MakeShareable(new FJsonValueNumber(V.Y)),
        MakeShareable(new FJsonValueNumber(V.Z))
        }));
}

FVector FVoxelPersistenceJsonUtils::JsonToVector(const TArray<TSharedPtr<FJsonValue>>& Array)
{
    if (Array.Num() < 3) return FVector::ZeroVector;
    return FVector(
        Array[0]->AsNumber(),
        Array[1]->AsNumber(),
        Array[2]->AsNumber()
    );
}

// --- 工具函数：FIntPoint <-> JSON Object ---
TSharedPtr<FJsonValue> FVoxelPersistenceJsonUtils::IntPointToJson(const FIntPoint& P)
{
    TSharedPtr<FJsonObject> Obj = MakeShareable(new FJsonObject);
    Obj->SetNumberField("X", P.X);
    Obj->SetNumberField("Y", P.Y);
    return MakeShareable(new FJsonValueObject(Obj));
}

FIntPoint FVoxelPersistenceJsonUtils::JsonToIntPoint(const TSharedPtr<FJsonObject>& Obj)
{
    if (!Obj.IsValid()) return FIntPoint::ZeroValue;
    int32 X = Obj->HasField("X") ? Obj->GetIntegerField("X") : 0;
    int32 Y = Obj->HasField("Y") ? Obj->GetIntegerField("Y") : 0;
    return FIntPoint(X, Y);
}

// --- FVoxelWorldMeta: ToJson ---
TSharedPtr<FJsonObject> FVoxelPersistenceJsonUtils::ToJson(const FVoxelWorldMeta& Meta)
{
    TSharedPtr<FJsonObject> Json = MakeShareable(new FJsonObject);
    Json->SetNumberField("Version", Meta.Version);
    Json->SetNumberField("Seed", Meta.Seed);
    Json->SetStringField("WorldName", Meta.WorldName);
    Json->SetArrayField("PlayerLocation", {
        MakeShareable(new FJsonValueNumber(Meta.PlayerLocation.X)),
        MakeShareable(new FJsonValueNumber(Meta.PlayerLocation.Y)),
        MakeShareable(new FJsonValueNumber(Meta.PlayerLocation.Z))
        });
    Json->SetArrayField("PlayerRotation", {
        MakeShareable(new FJsonValueNumber(Meta.PlayerRotation.Pitch)),
        MakeShareable(new FJsonValueNumber(Meta.PlayerRotation.Yaw)),
        MakeShareable(new FJsonValueNumber(Meta.PlayerRotation.Roll))
        });
    Json->SetStringField("LastSavedTime", Meta.LastSavedTime);

    TArray<TSharedPtr<FJsonValue>> ChunkList;
    for (const FIntPoint& P : Meta.ChunkList)
    {
        ChunkList.Add(IntPointToJson(P));
    }
    Json->SetArrayField("Chunks", ChunkList);

    return Json;
}

// --- FVoxelWorldMeta: FromJson ---
bool FVoxelPersistenceJsonUtils::FromJson(const TSharedPtr<FJsonObject>& Json, FVoxelWorldMeta& OutMeta)
{
    if (!Json.IsValid()) return false;

    OutMeta.Version = Json->HasField("Version") ? Json->GetIntegerField("Version") : 1;
    OutMeta.Seed = Json->HasField("Seed") ? Json->GetIntegerField("Seed") : 0;
    OutMeta.WorldName = Json->HasField("WorldName") ? Json->GetStringField("WorldName") : TEXT("Unknown");

    if (Json->HasField("PlayerLocation"))
    {
        const TArray<TSharedPtr<FJsonValue>>& LocArr = Json->GetArrayField("PlayerLocation");
        if (LocArr.Num() >= 3)
            OutMeta.PlayerLocation = JsonToVector(LocArr);
    }

    if (Json->HasField("PlayerRotation"))
    {
        const TArray<TSharedPtr<FJsonValue>>& RotArr = Json->GetArrayField("PlayerRotation");
        if (RotArr.Num() >= 3)
        {
            OutMeta.PlayerRotation.Pitch = RotArr[0]->AsNumber();
            OutMeta.PlayerRotation.Yaw = RotArr[1]->AsNumber();
            OutMeta.PlayerRotation.Roll = RotArr[2]->AsNumber();
        }
    }

    OutMeta.LastSavedTime = Json->HasField("LastSavedTime") ? Json->GetStringField("LastSavedTime") : TEXT("");

    OutMeta.ChunkList.Empty();
    if (Json->HasField("Chunks"))
    {
        const TArray<TSharedPtr<FJsonValue>>& Chunks = Json->GetArrayField("Chunks");
        for (const TSharedPtr<FJsonValue>& Val : Chunks)
        {
            if (Val->Type == EJson::Object)
            {
                OutMeta.ChunkList.Add(JsonToIntPoint(Val->AsObject()));
            }
        }
    }

    return true;
}

// --- FVoxelChunkData: ToJson ---
TSharedPtr<FJsonObject> FVoxelPersistenceJsonUtils::ToJson(const FVoxelChunkData& Chunk)
{
    TSharedPtr<FJsonObject> Json = MakeShareable(new FJsonObject);
    Json->SetNumberField("X", Chunk.ChunkCoordinate.X);
    Json->SetNumberField("Y", Chunk.ChunkCoordinate.Y);

    TArray<TSharedPtr<FJsonValue>> BlockValues;
    for (int32 ID : Chunk.VoxelData)
    {
        BlockValues.Add(MakeShareable(new FJsonValueNumber(ID)));
    }
    Json->SetArrayField("Blocks", BlockValues);

    return Json;
}

// --- FVoxelChunkData: FromJson ---
bool FVoxelPersistenceJsonUtils::FromJson(const TSharedPtr<FJsonObject>& Json, FVoxelChunkData& OutChunk)
{
    if (!Json.IsValid()) return false;

    OutChunk.ChunkCoordinate.X = Json->HasField("X") ? Json->GetIntegerField("X") : 0;
    OutChunk.ChunkCoordinate.Y = Json->HasField("Y") ? Json->GetIntegerField("Y") : 0;

    OutChunk.VoxelData.Empty();
    if (Json->HasField("Blocks"))
    {
        const TArray<TSharedPtr<FJsonValue>>& BlockArray = Json->GetArrayField("Blocks");
        for (const TSharedPtr<FJsonValue>& Value : BlockArray)
        {
            OutChunk.VoxelData.Add(static_cast<int32>(Value->AsNumber()));
        }
    }

    return true;
}

// --- 文件 I/O: Save ---
bool FVoxelPersistenceJsonUtils::SaveJsonToFile(const FString& FilePath, const TSharedPtr<FJsonObject>& Json)
{
    if (!Json.IsValid())
    {
        UE_LOG(H_LogVoxelPersistence, Error, TEXT("SaveJsonToFile: Json is null. Path=%s"), *FilePath);
        return false;
    }

    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    if (!FJsonSerializer::Serialize(Json.ToSharedRef(), Writer))
    {
        UE_LOG(H_LogVoxelPersistence, Error, TEXT("Failed to serialize JSON. Path=%s"), *FilePath);
        return false;
    }

    // 确保目录存在
    FString Dir = FPaths::GetPath(FilePath);
    IFileManager::Get().MakeDirectory(*Dir, true);

    if (!FFileHelper::SaveStringToFile(OutputString, *FilePath))
    {
        UE_LOG(H_LogVoxelPersistence, Error, TEXT("Failed to write file: %s"), *FilePath);
        return false;
    }

    UE_LOG(H_LogVoxelPersistence, Verbose, TEXT("Saved JSON to: %s"), *FilePath);
    return true;
}

// --- 文件 I/O: Load ---
bool FVoxelPersistenceJsonUtils::LoadJsonFromFile(const FString& FilePath, TSharedPtr<FJsonObject>& OutJson)
{
    FString FileContent;
    if (!FFileHelper::LoadFileToString(FileContent, *FilePath))
    {
        UE_LOG(H_LogVoxelPersistence, Warning, TEXT("File not found or unreadable: %s"), *FilePath);
        return false;
    }

    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(FileContent);
    OutJson = MakeShareable(new FJsonObject);
    if (!FJsonSerializer::Deserialize(Reader, OutJson) || !OutJson.IsValid())
    {
        UE_LOG(H_LogVoxelPersistence, Error, TEXT("Failed to parse JSON from: %s"), *FilePath);
        return false;
    }

    UE_LOG(H_LogVoxelPersistence, Verbose, TEXT("Loaded JSON from: %s"), *FilePath);
    return true;
}