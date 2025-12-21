#include "GameCoreModule.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_MODULE(FGameCore, GameCore);

void FGameCore::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("GameCore module has started!"));
}

void FGameCore::ShutdownModule()
{
	UE_LOG(LogTemp, Log, TEXT("GameCore module is shutting down."));
}
