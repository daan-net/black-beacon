#include "BlackBeacon/Power/BBGeneratorComponent.h"

#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Internationalization/Text.h"

#include "BlackBeacon/Objectives/BBObjectiveSystem.h"
#include "BlackBeacon/Power/BBPowerSystem.h"

UBBGeneratorComponent::UBBGeneratorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UBBGeneratorComponent::BeginPlay()
{
	Super::BeginPlay();

	// Register as a power source for this world's network.
	if (UWorld* const World = GetWorld())
	{
		if (UBBPowerSystem* const Power = World->GetSubsystem<UBBPowerSystem>())
		{
			Power->RegisterSource(this);
		}
	}
}

void UBBGeneratorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* const World = GetWorld())
	{
		if (UBBPowerSystem* const Power = World->GetSubsystem<UBBPowerSystem>())
		{
			Power->UnregisterSource(this);
		}
	}
	Super::EndPlay(EndPlayReason);
}

void UBBGeneratorComponent::Start()
{
	if (bRunning)
	{
		return;
	}
	SetRunning(true);
}

void UBBGeneratorComponent::Stop()
{
	if (!bRunning)
	{
		return;
	}
	SetRunning(false);
}

void UBBGeneratorComponent::RestoreState(bool bInRunning, float InSpinUpProgress, bool bInHasProducedOnce)
{
	const bool bWasRunning = bRunning;
	const bool bWasProducing = IsProducing();
	if (AActor* const Owner = GetOwner())
	{
		Owner->GetWorldTimerManager().ClearTimer(SpinUpTimerHandle);
	}

	bRunning = bInRunning;
	SpinUpProgress = bRunning ? FMath::Clamp(InSpinUpProgress, 0.0f, 1.0f) : 0.0f;
	bHasProducedOnce = bInHasProducedOnce;

	if (bRunning && SpinUpProgress < 1.0f)
	{
		if (AActor* const Owner = GetOwner())
		{
			Owner->GetWorldTimerManager().SetTimer(
				SpinUpTimerHandle, this, &UBBGeneratorComponent::AdvanceSpinUp,
				0.05f, true, 0.0f);
		}
	}

	if (bWasProducing != IsProducing())
	{
		OnGeneratorStateChanged.Broadcast(IsProducing());
	}
	if (bWasRunning != bRunning)
	{
		OnGeneratorRunningChanged.Broadcast(bRunning);
	}
	NotifyPowerNetworkChanged();
}

void UBBGeneratorComponent::SetRunning(bool bNowRunning)
{
	bRunning = bNowRunning;
	OnGeneratorRunningChanged.Broadcast(bRunning);

	AActor* const Owner = GetOwner();
	if (!Owner || !Owner->GetWorld())
	{
		return;
	}

	if (bRunning)
	{
		// Spin-up timer drives the physical start curve.
		Owner->GetWorldTimerManager().SetTimer(
			SpinUpTimerHandle, this, &UBBGeneratorComponent::AdvanceSpinUp,
			0.05f, /*bLoop=*/true, /*InitialDelay=*/0.0f);
	}
	else
	{
		Owner->GetWorldTimerManager().ClearTimer(SpinUpTimerHandle);
		const bool bWasProducing = SpinUpProgress >= 1.0f;
		SpinUpProgress = 0.0f;
		NotifyPowerNetworkChanged();
		if (bWasProducing)
		{
			OnGeneratorStateChanged.Broadcast(false);
		}
	}
}

void UBBGeneratorComponent::AdvanceSpinUp()
{
	if (!bRunning)
	{
		return;
	}

	if (SpinUpProgress < 1.0f)
	{
		// Linear ramp for 0.1; replace with a physical torque curve later.
		SpinUpProgress = FMath::Min(1.0f, SpinUpProgress + 0.05f / FMath::Max(SpinUpSeconds, 0.05f));
		if (SpinUpProgress >= 1.0f)
		{
			if (!bHasProducedOnce)
			{
				bHasProducedOnce = true;
				CompleteStartObjectiveIfNew();
			}
			OnGeneratorStateChanged.Broadcast(true);
		}
		NotifyPowerNetworkChanged();
	}
	else
	{
		SpinUpProgress = 1.0f;
		if (AActor* const Owner = GetOwner())
		{
			Owner->GetWorldTimerManager().ClearTimer(SpinUpTimerHandle);
		}

	}
}

void UBBGeneratorComponent::NotifyPowerNetworkChanged()
{
	if (UWorld* const World = GetWorld())
	{
		if (UBBPowerSystem* const Power = World->GetSubsystem<UBBPowerSystem>())
		{
			Power->RecalculateNetwork();
		}
	}
}

void UBBGeneratorComponent::CompleteStartObjectiveIfNew()
{
	if (UWorld* const World = GetWorld())
	{
		if (UGameInstance* const GI = World->GetGameInstance())
		{
			if (UBBObjectiveSystem* const Objectives = GI->GetSubsystem<UBBObjectiveSystem>())
			{
				Objectives->CompleteObjective(ObjectiveOnStarted.ToString());
			}
		}
	}
}

FText UBBGeneratorComponent::GetInteractionPrompt() const
{
	return bRunning
		? NSLOCTEXT("BlackBeacon", "GeneratorStopPrompt", "Stop Generator")
		: NSLOCTEXT("BlackBeacon", "GeneratorStartPrompt", "Start Generator");
}

void UBBGeneratorComponent::OnInteract(APlayerController* InteractingController)
{
	if (bRunning)
	{
		Stop();
	}
	else
	{
		Start();
	}
}
