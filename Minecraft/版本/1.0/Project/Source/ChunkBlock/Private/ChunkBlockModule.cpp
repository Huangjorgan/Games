#include "ChunkBlockModule.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_MODULE(FChunkBlock, ChunkBlock);

void FChunkBlock::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("ChunkBlock module has started!"));
}

void FChunkBlock::ShutdownModule()
{
	UE_LOG(LogTemp, Log, TEXT("ChunkBlock module is shutting down."));
}

