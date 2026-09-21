#include "BlackBeacon/Core/BBGameState.h"

#include "Engine/World.h"

#include "BlackBeacon/Objectives/BBObjectiveSystem.h"
#include "BlackBeacon/Power/BBPowerSystem.h"

void ABBlackBeaconGameState::BeginPlay()
{
	Super::BeginPlay();
	Subscribe();
}

void ABBlackBeaconGameState::Subscribe()
{
	if (UWorld* const World = GetWorld())
	{
		if (UGameInstance* const GI = World->GetGameInstance())
		{
			if (UBBObjectiveSystem* const Objectives = GI->GetSubsystem<UBBObjectiveSystem>())
			{
				Objectives->OnCurrentObjectiveChanged.AddUObject(this, &ABBlackBeaconGameState::HandleCurrentObjectiveChanged);
				HandleCurrentObjectiveChanged(Objectives->GetCurrentObjectiveId(), Objectives->GetCurrentObjectiveText());
			}
		}

		if (UBBPowerSystem* const Power = World->GetSubsystem<UBBPowerSystem>())
		{
			Power->OnConsumerPowerChanged.AddUObject(this, &ABBlackBeaconGameState::HandleConsumerPowerChanged);
		}
	}
}

void ABBlackBeaconGameState::HandleCurrentObjectiveChanged(const FString& ObjectiveId, const FString& ObjectiveText)
{
	CurrentObjectiveId = ObjectiveId;
	CurrentObjectiveText = ObjectiveText;
	OnUiObjectiveChanged.Broadcast(ObjectiveId, ObjectiveText);
}

void ABBlackBeaconGameState::HandleConsumerPowerChanged(const FName& ConsumerId, bool bPowered)
{
	if (ConsumerId == FName(TEXT("lighthouse_lantern")))
	{
		bLighthousePowered = bPowered;
		OnUiPowerChanged.Broadcast(bPowered);
	}
}