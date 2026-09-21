// BLACK BEACON - objective trigger component.
//
// A generic bridge between world/event signals and the objective system:
//
//   EnterVolume : completes ObjectiveId when the local player pawn enters
//                 an owner actor's overlap volume.
//
// The interaction-driven beats (generator, lighthouse) complete objectives
// directly from their OnInteract paths - no volume required.
//
// This keeps objectives data-driven: the builder places volumes and
// configures IDs; no gameplay code knows the chain order.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "BBObjectiveTriggerComponent.generated.h"

UENUM(BlueprintType)
enum class EBBObjectiveTriggerType : uint8
{
	EnterVolume,
	Event // fired manually from code when an event happens
};

UCLASS(ClassGroup = (BlackBeacon), meta = (BlueprintSpawnableComponent))
class UBBObjectiveTriggerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBBObjectiveTriggerComponent();

	UPROPERTY(EditAnywhere, Category = "BlackBeacon|Objective")
	FName ObjectiveId;

	UPROPERTY(EditAnywhere, Category = "BlackBeacon|Objective")
	EBBObjectiveTriggerType TriggerType = EBBObjectiveTriggerType::EnterVolume;

	// EnterVolume: only the local player pawn triggers completion.
	UPROPERTY(EditAnywhere, Category = "BlackBeacon|Objective")
	bool bPlayerOnly = true;

	// Manual fire for Event type triggers.
	UFUNCTION(BlueprintCallable, Category = "BlackBeacon|Objective")
	void Fire();

	// Delegate that allows code to complete other objectives on demand
	// (e.g. "restore power" fires when the lighthouse first becomes powered).
	DECLARE_MULTICAST_DELEGATE_OneParam(FBBObjectiveTriggered, const FName& /*ObjectiveId*/);
	FBBObjectiveTriggered OnObjectiveTriggered;

protected:
	virtual void OnRegister() override;
	virtual void OnUnregister() override;

private:
	UFUNCTION()
	void HandleActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);

	void RequestCompletion() const;
};