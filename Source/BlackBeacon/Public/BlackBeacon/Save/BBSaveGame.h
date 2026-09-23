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

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "BlackBeacon|Save")
	bool bGeneratorRunning = false;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "BlackBeacon|Save")
	float GeneratorSpinUpProgress = 0.0f;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "BlackBeacon|Save")
	bool bGeneratorHasProducedOnce = false;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "BlackBeacon|Save")
	bool bLighthousePowered = false;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "BlackBeacon|Save")
	bool bBeamRunning = false;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "BlackBeacon|Save")
	bool bBeamStarted = false;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "BlackBeacon|Save")
	int32 BeamRotationMode = 0; // EBBBeamRotationMode as int

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "BlackBeacon|Save")
	bool bAimObjectiveCompleted = false;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "BlackBeacon|Save")
	float BeamYawDegrees = 0.0f;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "BlackBeacon|Save")
	float BeamPitchDegrees = 0.0f;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "BlackBeacon|Save")
	TArray<FString> CompletedObjectives;

	// Actors revealed permanently (persistent anomalies stay revealed).
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "BlackBeacon|Save")
	TArray<FString> PersistentlyRevealedActors;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "BlackBeacon|Save")
	int32 WeatherPhase = 0;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "BlackBeacon|Save")
	bool bHasPlayerTransform = false;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "BlackBeacon|Save")
	FVector PlayerLocation = FVector::ZeroVector;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "BlackBeacon|Save")
	FRotator PlayerRotation = FRotator::ZeroRotator;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "BlackBeacon|Save")
	float PlayTimeSeconds = 0.0f;
};

UCLASS()
class UBBSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(SaveGame, VisibleAnywhere, Category = "BlackBeacon|Save")
	FBBWorldSaveData WorldData;
};
