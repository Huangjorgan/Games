#include "WorldGenerationModule.h"
#include "Modules/ModuleManager.h"
#include "LogWorldGeneration.h"

DEFINE_LOG_CATEGORY(H_LogWorldGeneration);

IMPLEMENT_MODULE(FWorldGeneration, WorldGeneration);

void FWorldGeneration::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("WorldGeneration module has started."));
}

void FWorldGeneration::ShutdownModule()
{
	UE_LOG(LogTemp, Log, TEXT("WorldGeneration module has shut down."));
}
