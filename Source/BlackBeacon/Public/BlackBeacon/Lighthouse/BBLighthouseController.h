// BLACK BEACON - lighthouse controller.
//
// The lighthouse as a gameplay actor: owns the beam component, is the
// network's power consumer, and is the "START LIGHTHOUSE" interactable.
// Visuals (tower, lens, machinery) are children of this actor (greybox
// in 0.1, real assets later) - gameplay never depends on the meshes.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "BlackBeacon/Interaction/BBInteractableInterface.h"
#include "BlackBeacon/Power/BBPowerConsumerInterface.h"

#include "BBLighthouseController.generated.h"

class UBBLighthouseBeamComponent;

UCLASS()
class ABBLighthouseController : public AActor,
	public IBBPowerConsumerInterface,
	public IBBInteractableInterface
{
	GENERATED_BODY()

public:
	ABBLighthouseController();

	// --- components ---
	UPROPERTY(VisibleAnywhere, Category = "BlackBeacon|Lighthouse")
	TObjectPtr<UBBLighthouseBeamComponent> BeamComponent = nullptr;

	// --- power consumer ---
	virtual FName GetConsumerId() const override;
	virtual float GetDemandWatts() const override;
	virtual void NotifyPowerState(bool bPoweredNow, float SuppliedWatts) override;

	// --- interactable ---
	virtual FText GetInteractionPrompt() const override;
	virtual void OnInteract(APlayerController* InteractingController) override;

	// --- API ---
	UFUNCTION(BlueprintCallable, Category = "BlackBeacon|Lighthouse")
	void RequestBeamStart();

	UFUNCTION(BlueprintCallable, Category = "BlackBeacon|Lighthouse")
	bool IsPowered() const { return bPowered; }

	// True while the player is manning the beam (manual aim mode).
	UFUNCTION(BlueprintCallable, Category = "BlackBeacon|Lighthouse")
	bool IsBeamInManualMode() const;

	// --- config ---
	// Beam only spins up when the player requests it (objective pacing).
	UPROPERTY(config, EditAnywhere, Category = "BlackBeacon|Lighthouse")
	bool bAutoStartBeamWhenPowered = false;

	UPROPERTY(EditAnywhere, Category = "BlackBeacon|Lighthouse")
	FName ConsumerId = TEXT("lighthouse_lantern");

	UPROPERTY(EditAnywhere, Category = "BlackBeacon|Lighthouse")
	float DemandWatts = 8000.0f;

	UPROPERTY(EditAnywhere, Category = "BlackBeacon|Lighthouse")
	FName ObjectiveOnStarted = TEXT("BB_OBJ_START_LIGHTHOUSE");

	UPROPERTY(EditAnywhere, Category = "BlackBeacon|Lighthouse")
	FName ObjectiveOnPowered = TEXT("BB_OBJ_RESTORE_POWER");

	UPROPERTY(EditAnywhere, Category = "BlackBeacon|Lighthouse")
	FName ObjectiveOnAim = TEXT("BB_OBJ_AIM_BEAM");

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void RegisterWithPowerSystem();
	void HandleFirstPower(float SuppliedWatts);
	void ToggleBeamControl();

	bool bPowered = false;
	bool bPowerGrantedOnce = false;
	bool bBeamStarted = false;
	bool bAimObjectiveCompleted = false;
};