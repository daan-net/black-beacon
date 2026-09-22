#include "BlackBeacon/Core/BBGameMode.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"

#include "BlackBeacon/Core/BBGameState.h"
#include "BlackBeacon/Core/BBPlayerCharacter.h"
#include "BlackBeacon/Core/BBPlayerController.h"
#include "BlackBeacon/Core/BBProceduralWorld.h"
#include "BlackBeacon/Objectives/BBObjectiveSystem.h"

ABBlackBeaconGameMode::ABBlackBeaconGameMode()
{
	PlayerControllerClass = ABBlackBeaconPlayerController::StaticClass();
	DefaultPawnClass = ABBlackBeaconPlayerCharacter::StaticClass();
	GameStateClass = ABBlackBeaconGameState::StaticClass();
}

void ABBlackBeaconGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (bBuildProceduralBootstrapWorld)
	{
		BuildProceduralSlice();
	}

	if (!bArrivalCompleted)
	{
		bArrivalCompleted = true;
		if (UGameInstance* const GI = GetGameInstance())
		{
			if (UBBObjectiveSystem* const Objectives = GI->GetSubsystem<UBBObjectiveSystem>())
			{
				Objectives->CompleteObjective(ArrivalObjective.ToString());
			}
		}
	}
}

void ABBlackBeaconGameMode::BuildProceduralSlice()
{
	UBBProceduralWorld::BuildSlice(GetWorld());
}

AActor* ABBlackBeaconGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	// The bootstrap slice owns its landing point even when Entry has a
	// built-in PlayerStart at the map origin.
	if (bBuildProceduralBootstrapWorld
		&& (!BootstrapPlayerStart.GetLocation().IsNearlyZero() || !BootstrapPlayerStart.GetRotation().IsIdentity()))
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		APlayerStart* const Start = GetWorld()->SpawnActor<APlayerStart>(
			APlayerStart::StaticClass(), BootstrapPlayerStart, Params);
		if (Start)
		{
			return Start;
		}
	}

	TActorIterator<APlayerStart> ExistingStart(GetWorld());
	if (ExistingStart)
	{
		return *ExistingStart;
	}

	return Super::ChoosePlayerStart_Implementation(Player);
}
