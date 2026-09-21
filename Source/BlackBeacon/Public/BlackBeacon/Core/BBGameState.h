// BLACK BEACON - game state.
//
// UI-facing snapshot of the slice: current objective and power/beam state.
// It subscribes to the objective and power subsystems and re-broadcasts in
// UE-friendly form for HUD widgets. Kept deliberately thin - it holds no
// simulation state of its own.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"

#include "BBGameState.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FBBUiObjectiveChanged, const FString& /*ObjectiveId*/, const FString& /*ObjectiveText*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FBBUiBoolChanged, bool /*bValue*/);

UCLASS()
class ABBlackBeaconGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	// --- live snapshots (read by HUD) ---
	UFUNCTION(BlueprintCallable, Category = "BlackBeacon|UI")
	FString GetCurrentObjectiveId() const { return CurrentObjectiveId; }

	UFUNCTION(BlueprintCallable, Category = "BlackBeacon|UI")
	FString GetCurrentObjectiveText() const { return CurrentObjectiveText; }

	UFUNCTION(BlueprintCallable, Category = "BlackBeacon|UI")
	bool IsLighthousePowered() const { return bLighthousePowered; }

	UFUNCTION(BlueprintCallable, Category = "BlackBeacon|UI")
	bool IsBeamRunning() const { return bBeamRunning; }

	// --- events for HUD widgets ---
	FBBUiObjectiveChanged OnUiObjectiveChanged;
	FBBUiBoolChanged OnUiPowerChanged;

private:
	void Subscribe();
	void HandleCurrentObjectiveChanged(const FString& ObjectiveId, const FString& ObjectiveText);
	void HandleConsumerPowerChanged(const FName& ConsumerId, bool bPowered);

	UPROPERTY()
	FString CurrentObjectiveId;

	UPROPERTY()
	FString CurrentObjectiveText;

	bool bLighthousePowered = false;
	bool bBeamRunning = false;
};