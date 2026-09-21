// BLACK BEACON - game mode.
//
// Owner of the runtime defaults (pawn/controller/game state) plus the
// greybox bootstrap: when bBuildProceduralBootstrapWorld is set (default),
// it builds the 0.1 slice from engine basic shapes at runtime so the
// project runs with zero authored .uasset dependencies. Authored maps in
// later milestones disable the builder and place real actors instead.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

#include "BBGameMode.generated.h"

UCLASS(config = Game)
class ABBlackBeaconGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ABBlackBeaconGameMode();

	virtual void BeginPlay() override;

	// Ensures a player start exists even on a blank map (deferred-spawn
	// friendly): falls back to the configured bootstrap transform.
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	UPROPERTY(config, EditAnywhere, Category = "BlackBeacon|Bootstrap")
	bool bBuildProceduralBootstrapWorld = true;

	// Used when the map has no player start (runtime greybox slice).
	UPROPERTY(config, EditAnywhere, Category = "BlackBeacon|Bootstrap")
	FTransform BootstrapPlayerStart;

	UPROPERTY(EditAnywhere, Category = "BlackBeacon|Bootstrap")
	FName ArrivalObjective = TEXT("BB_OBJ_ARRIVE");

protected:
	// Builds weather + lighthouse + beach + tower + annex + reveal object.
	// Implemented in BBProceduralWorld.cpp (kept out of the game mode so
	// the builder can grow independently).
	void BuildProceduralSlice();

private:
	bool bArrivalCompleted = false;
};