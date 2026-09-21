// BLACK BEACON - save/load subsystem (foundation).
//
// A thin seam around USaveGame slots. 0.1 exposes save/load of the world
// snapshot struct; game code decides WHEN to snapshot (0.2 introduces
// checkpoints; the data contract below is stable).

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "BBSaveGame.h"

#include "BBSaveSubsystem.generated.h"

UCLASS()
class UBBSaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Writes the snapshot into the save game (player slot).
	UFUNCTION(BlueprintCallable, Category = "BlackBeacon|Save")
	bool SaveWorldData(const FBBWorldSaveData& WorldData, const FString& SlotName = TEXT(""));

	// Returns true and fills OutData when a save exists for the slot.
	UFUNCTION(BlueprintCallable, Category = "BlackBeacon|Save")
	bool LoadWorldData(FBBWorldSaveData& OutWorldData, const FString& SlotName = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "BlackBeacon|Save")
	bool HasSaveData(const FString& SlotName = TEXT("")) const;

	// Convenience: gather the current world snapshot from live systems.
	// (Implemented in 0.2 when checkpointing lands; the subsystem owns the
	//  piecewise writers so gameplay code stays decoupled.)
	static FBBWorldSaveData BuildSnapshot(class UWorld* World);

private:
	static FString ResolveSlot(const FString& SlotName);
};