// BLACK BEACON - save game foundation.
//
// 0.1 scope: the data shapes exist and the save/load seam is wired through
// UBBSaveSubsystem. Full save-checkpoint integration is 0.2.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"

#include "BBSaveGame.generated.h"

// Everything a save must remember about the slice's world state.
USTRUCT(BlueprintType)
struct FBBWorldSaveData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "BlackBeacon|Save")
	bool bGeneratorRunning = false;

	UPROPERTY(BlueprintReadWrite, Category = "BlackBeacon|Save")
	bool bLighthousePowered = false;

	UPROPERTY(BlueprintReadWrite, Category = "BlackBeacon|Save")
	bool bBeamRunning = false;

	UPROPERTY(BlueprintReadWrite, Category = "BlackBeacon|Save")
	int32 BeamRotationMode = 0; // EBBBeamRotationMode as int

	UPROPERTY(BlueprintReadWrite, Category = "BlackBeacon|Save")
	TArray<FString> CompletedObjectives;

	// Actors revealed permanently (persistent anomalies stay revealed).
	UPROPERTY(BlueprintReadWrite, Category = "BlackBeacon|Save")
	TArray<FString> PersistentlyRevealedActors;

	UPROPERTY(BlueprintReadWrite, Category = "BlackBeacon|Save")
	float WeatherPhase = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "BlackBeacon|Save")
	float PlayTimeSeconds = 0.0f;
};

UCLASS()
class UBBSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, Category = "BlackBeacon|Save")
	FBBWorldSaveData WorldData;
};