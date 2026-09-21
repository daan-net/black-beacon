#include "BlackBeacon/Core/BBGameMode.h"

#include "Engine/World.h"
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
	if (AActor* const Found = FindPlayerStart(Player))
	{
		return Found;
	}

	// Blank map (Entry, or an empty test level): synthesize a player start
	// at the configured bootstrap transform so the slice always boots.
	if (!BootstrapPlayerStart.GetLocation().IsNearlyZero() || !BootstrapPlayerStart.GetRotation().IsIdentity())
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

	return Super::ChoosePlayerStart_Implementation(Player);
}