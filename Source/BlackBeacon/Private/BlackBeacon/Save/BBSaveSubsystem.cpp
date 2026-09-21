#include "BlackBeacon/Save/BBSaveSubsystem.h"

#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"

namespace
{
	constexpr const TCHAR* kDefaultSaveSlot = TEXT("slot_blackbeacon_0");
	FString ResolveSlotInternal(const FString& SlotName)
	{
		return SlotName.IsEmpty() ? FString(kDefaultSaveSlot) : SlotName;
	}
}

FString UBBSaveSubsystem::ResolveSlot(const FString& SlotName)
{
	return ResolveSlotInternal(SlotName);
}

bool UBBSaveSubsystem::SaveWorldData(const FBBWorldSaveData& WorldData, const FString& SlotName)
{
	UBBSaveGame* const Save = Cast<UBBSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UBBSaveGame::StaticClass()));
	if (!Save)
	{
		return false;
	}

	Save->WorldData = WorldData;
	return UGameplayStatics::SaveGameToSlot(Save, ResolveSlotInternal(SlotName), 0);
}

bool UBBSaveSubsystem::LoadWorldData(FBBWorldSaveData& OutWorldData, const FString& SlotName)
{
	USaveGame* const Loaded = UGameplayStatics::LoadGameFromSlot(ResolveSlotInternal(SlotName), 0);
	UBBSaveGame* const Save = Cast<UBBSaveGame>(Loaded);
	if (!Save)
	{
		return false;
	}

	OutWorldData = Save->WorldData;
	return true;
}

bool UBBSaveSubsystem::HasSaveData(const FString& SlotName) const
{
	return UGameplayStatics::DoesSaveGameExist(ResolveSlotInternal(SlotName), 0);
}

FBBWorldSaveData UBBSaveSubsystem::BuildSnapshot(UWorld* World)
{
	// 0.2: read live generator / lighthouse / objective / weather state into
	// the snapshot. The shape is already frozen by FBBWorldSaveData above.
	return FBBWorldSaveData();
}