// BLACK BEACON - objective system (game-instance subsystem).
//
// Holds the objective graph defined in Config/DefaultGame.ini and resolves
// activations/completions event-driven. The chain definition is TEXT -
// gameplay code never hard-codes sequence; it just requests completions
// with objective IDs. Data-driven by design.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "BlackBeacon/Logics/BBObjectiveGraph.h"

#include "BBObjectiveSystem.generated.h"

// One node of the config-driven objective chain.
USTRUCT()
struct FBBObjectiveNodeConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "BlackBeacon|Objective")
	FString Id;

	UPROPERTY(EditAnywhere, Category = "BlackBeacon|Objective")
	FString Text;

	// IDs that must be completed before this node can be completed.
	UPROPERTY(EditAnywhere, Category = "BlackBeacon|Objective")
	TArray<FString> Prereqs;
};

DECLARE_MULTICAST_DELEGATE_TwoParams(FBBObjectiveCompleted, const FString& /*ObjectiveId*/, bool /*bNewlyCompleted*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FBBObjectiveChanged, const FString& /*ObjectiveId*/, const FString& /*ObjectiveText*/);

UCLASS(config = Game)
class UBBObjectiveSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// --- API ---
	UFUNCTION(BlueprintCallable, Category = "BlackBeacon|Objectives")
	bool ActivateObjective(const FString& ObjectiveId);

	UFUNCTION(BlueprintCallable, Category = "BlackBeacon|Objectives")
	bool CompleteObjective(const FString& ObjectiveId);

	bool IsCompleted(const FString& ObjectiveId) const;
	bool IsActive(const FString& ObjectiveId) const;
	bool IsFinished() const;

	UFUNCTION(BlueprintCallable, Category = "BlackBeacon|Objectives")
	FString GetCurrentObjectiveId() const;

	UFUNCTION(BlueprintCallable, Category = "BlackBeacon|Objectives")
	FString GetCurrentObjectiveText() const;

	// --- events ---
	// Fired when any objective completes (bNewlyCompleted for the first time).
	FBBObjectiveCompleted OnObjectiveCompleted;
	// Fired when the living "current" objective changes.
	FBBObjectiveChanged OnCurrentObjectiveChanged;

private:
	void BuildGraphFromConfig();
	void PublishCurrent();

	UPROPERTY(config, EditAnywhere, Category = "BlackBeacon|Objectives")
	TArray<FBBObjectiveNodeConfig> ObjectiveChain;

	BlackBeacon::Logics::FBBObjectiveGraph Graph;
	bool bGraphBuilt = false;
	bool bLastFinished = false;
};