
#include "BlockSubsystem.h"
#include "BlockType.h"
#include "Engine/World.h"

void UBlockSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	BlocksByID.Empty();
	BlocksByName.Empty();
}

void UBlockSubsystem::Deinitialize()
{
	BlocksByID.Empty();
	BlocksByName.Empty();
	Super::Deinitialize();
}

void UBlockSubsystem::RegisterBlock(UBlockType* Block)
{
	if (!Block)
		return;

	if (Block->BlockID != 0)
	{
		BlocksByID.Add(Block->BlockID, Block);
	}
	if (!Block->BlockName.IsNone())
	{
		BlocksByName.Add(Block->BlockName, Block);
	}
}

UBlockType* UBlockSubsystem::FindBlockByID(int32 BlockID) const
{
	if (const TWeakObjectPtr<UBlockType>* Found = BlocksByID.Find(BlockID))
	{
		return Found->Get();
	}
	return nullptr;
}

UBlockType* UBlockSubsystem::FindBlockByName(FName Name) const
{
	if (const TWeakObjectPtr<UBlockType>* Found = BlocksByName.Find(Name))
	{
		return Found->Get();
	}
	return nullptr;
}

