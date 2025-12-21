#include "VoxelPersistenceModule.h"
#include "Modules/ModuleManager.h"
#include "LogVoxelPersistence.h"

DEFINE_LOG_CATEGORY(H_LogVoxelPersistence);

IMPLEMENT_MODULE(FVoxelPersistence, VoxelPersistence);

void FVoxelPersistence::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("VoxelPersistence module has started!"));

}

void FVoxelPersistence::ShutdownModule()
{
	UE_LOG(LogTemp, Log, TEXT("VoxelPersistence module has shut down."));
}
