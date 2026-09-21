#include "BlackBeacon/Objectives/BBObjectiveTriggerComponent.h"

#include "Engine/World.h"
#include "GameFramework/Pawn.h"

#include "BlackBeacon/Objectives/BBObjectiveSystem.h"

UBBObjectiveTriggerComponent::UBBObjectiveTriggerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UBBObjectiveTriggerComponent::OnRegister()
{
	Super::OnRegister();

	if (TriggerType == EBBObjectiveTriggerType::EnterVolume)
	{
		if (AActor* const Owner = GetOwner())
		{
			Owner->OnActorBeginOverlap.AddDynamic(this, &UBBObjectiveTriggerComponent::HandleActorBeginOverlap);
		}
	}
}

void UBBObjectiveTriggerComponent::OnUnregister()
{
	if (AActor* const Owner = GetOwner())
	{
		Owner->OnActorBeginOverlap.RemoveDynamic(this, &UBBObjectiveTriggerComponent::HandleActorBeginOverlap);
	}
	Super::OnUnregister();
}

void UBBObjectiveTriggerComponent::HandleActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	if (!OtherActor)
	{
		return;
	}

	if (bPlayerOnly)
	{
		const APawn* const Pawn = Cast<APawn>(OtherActor);
		if (!Pawn || !Pawn->IsLocallyControlled())
		{
			return;
		}
	}

	RequestCompletion();
}

void UBBObjectiveTriggerComponent::Fire()
{
	RequestCompletion();
}

void UBBObjectiveTriggerComponent::RequestCompletion() const
{
	if (ObjectiveId.IsNone())
	{
		return;
	}

	if (UWorld* const World = GetWorld())
	{
		if (UGameInstance* const GI = World->GetGameInstance())
		{
			if (UBBObjectiveSystem* const Objectives = GI->GetSubsystem<UBBObjectiveSystem>())
			{
				Objectives->CompleteObjective(ObjectiveId.ToString());
			}
		}
	}

	OnObjectiveTriggered.Broadcast(ObjectiveId);
}