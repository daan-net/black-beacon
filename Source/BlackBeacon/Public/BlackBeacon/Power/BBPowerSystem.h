// BLACK BEACON - power network (world subsystem).
//
// A deliberately small grid: sources register, consumers register, and the
// network recalculates ONLY when something changes (registration or a
// source's state flips). No per-frame polling. 0.1 has one source
// (generator) and one consumer (lighthouse) but the registry is generic.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"

#include "BBPowerSystem.generated.h"

class IBBPowerSourceInterface;
class IBBPowerConsumerInterface;

DECLARE_MULTICAST_DELEGATE_TwoParams(FBBPowerConsumerChanged, const FName& /*ConsumerId*/, bool /*bPowered*/);

UCLASS()
class UBBPowerSystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// --- registration (components call this in BeginPlay/EndPlay) ---
	void RegisterSource(const TScriptInterface<IBBPowerSourceInterface>& Source);
	void UnregisterSource(const TScriptInterface<IBBPowerSourceInterface>& Source);
	void RegisterConsumer(const TScriptInterface<IBBPowerConsumerInterface>& Consumer);
	void UnregisterConsumer(const TScriptInterface<IBBPowerConsumerInterface>& Consumer);

	// Re-runs the supply/demand calculation and notifies consumers whose
	// state changed. Called automatically on register/unregister and after
	// any source state change.
	void RecalculateNetwork();

	bool IsConsumerPowered(const FName& ConsumerId) const;
	float GetSuppliedWatts(const FName& ConsumerId) const;
	float GetTotalAvailableWatts() const;

	// Broadcast for every consumer whose powered state changed.
	FBBPowerConsumerChanged OnConsumerPowerChanged;

private:
	void ClearCache();

	TArray<TScriptInterface<IBBPowerSourceInterface>> Sources;
	TArray<TScriptInterface<IBBPowerConsumerInterface>> Consumers;
	TMap<FName, float> SuppliedWattsByConsumer;
};