// BLACK BEACON - procedural greybox world builder.
//
// Builds the 0.1 vertical slice at runtime from engine basic-shape meshes:
//   beach/landing, ocean, storm-set weather, lighthouse tower with a
//   helical stair to the lantern room, generator annex, trigger volumes for
//   the objective chain, the beam, and one Beam-Reveal anomaly.
//
// This is DEV TOOLING: a runnable slice with zero authored assets so the
// project boots on a fresh engine install. Real maps replace it wholesale
// in milestone 0.2 (gameplay stays because it lives in components/actors).

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "BBProceduralWorld.generated.h"

class ABBLighthouseController;
class ABBWeatherController;

UCLASS()
class UBBProceduralWorld : public UObject
{
	GENERATED_BODY()

public:
	// Entry point: spawns the whole greybox slice. Returns the number of
	// actors spawned (sanity/coverage reporting for tests).
	static int32 BuildSlice(UWorld* World);

	// Individual pieces (independent so an authored map could call these
	// from an editor utility later).
	static ABBWeatherController* EnsureWeatherController(UWorld* World);

private:
	// --- helpers ---
	static AActor* SpawnMeshActor(
		UWorld* World,
		const TCHAR* BasicShapePath,
		const FTransform& Transform,
		FVector MeshScale,
		const FName& Tag);

	static AActor* SpawnTriggerVolume(
		UWorld* World,
		const FName& ObjectiveId,
		const FVector& Center,
		const FVector& HalfExtent,
		bool bPlayerOnly);

	static ABBLighthouseController* SpawnLighthouse(UWorld* World);
	static void SpawnGeneratorAnnex(UWorld* World);
	static void SpawnTowerAndStairs(UWorld* World);
	static void SpawnRevealAnomaly(UWorld* World);

	static constexpr float kTowerFloorHeightCm = 520.0f;
	static constexpr float kTowerRadiusCm = 320.0f;
};