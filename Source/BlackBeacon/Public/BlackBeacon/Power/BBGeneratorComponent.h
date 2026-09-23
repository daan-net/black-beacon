// BLACK BEACON - generator component.
//
// The engine-annex generator: interact to start it, it spins up over a
// configurable delay (physically believable start), and while running it
// is a power source in the network. Also an interactable, and completes
// an objective when first started.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "BlackBeacon/Interaction/BBInteractableInterface.h"
#include "BlackBeacon/Power/BBPowerSourceInterface.h"

#include "BBGeneratorComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FBBGeneratorStateChanged, bool /*bRunning*/);

UCLASS(ClassGroup = (BlackBeacon), config = Game, meta = (BlueprintSpawnableComponent))
class UBBGeneratorComponent : public UActorComponent,
	public IBBInteractableInterface,
	public IBBPowerSourceInterface
{
	GENERATED_BODY()

public:
	UBBGeneratorComponent();

	// --- control ---
	UFUNCTION(BlueprintCallable, Category = "BlackBeacon|Generator")
	void Start();

	UFUNCTION(BlueprintCallable, Category = "BlackBeacon|Generator")
	void Stop();

	bool IsRunning() const { return bRunning; }
	bool IsProducing() const { return bRunning && SpinUpProgress >= 1.0f; }
	float GetSpinUpProgress() const { return SpinUpProgress; }
	bool HasProducedOnce() const { return bHasProducedOnce; }
	void RestoreState(bool bInRunning, float InSpinUpProgress, bool bInHasProducedOnce);

	// Fired on every running-state change (bRunning true when it starts
	// producing, false when it stops).
	FBBGeneratorStateChanged OnGeneratorStateChanged;

	// --- power source ---
	virtual bool IsSourceActive() const override { return IsProducing(); }
	virtual float GetCurrentWatts() const override { return bRunning ? MaxWatts * SpinUpProgress : 0.0f; }
	virtual FName GetSourceId() const override { return SourceId; }

	// --- interactable ---
	virtual FText GetInteractionPrompt() const override;
	virtual void OnInteract(APlayerController* InteractingController) override;

	// --- config ---
	UPROPERTY(config, EditAnywhere, Category = "BlackBeacon|Generator")
	float SpinUpSeconds = 4.0f;

	UPROPERTY(config, EditAnywhere, Category = "BlackBeacon|Generator")
	float MaxWatts = 12000.0f;

	UPROPERTY(EditAnywhere, Category = "BlackBeacon|Generator")
	FName SourceId = TEXT("generator_main");

	// Objective completed the moment the generator first produces power.
	UPROPERTY(EditAnywhere, Category = "BlackBeacon|Generator")
	FName ObjectiveOnStarted = TEXT("BB_OBJ_START_GENERATOR");

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void AdvanceSpinUp(); // timer callback
	void SetRunning(bool bNowRunning);
	void NotifyPowerNetworkChanged();
	void CompleteStartObjectiveIfNew();

	bool bRunning = false;
	float SpinUpProgress = 0.0f; // 0..1 (ramps up / coasts down)
	FTimerHandle SpinUpTimerHandle;
	bool bHasProducedOnce = false;
};
