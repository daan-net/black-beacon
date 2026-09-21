// BLACK BEACON - beam reveal component.
//
// Implements the signature anomaly mechanic on any actor: invisible by
// default; the lighthouse beam rests on it and it materialises; the beam
// moves on and it fades (or stays, when persistent).
//
// A thin UE adapter over the engine-free FBBRevealMachine (unit-tested in
// Tests/). It subscribes to the world's beam automatically and applies the
// machine's visibility amount to its owner actor. All tuning is per-instance
// config (Config/DefaultGame.ini defaults).

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "BlackBeacon/Logics/BBRevealStateMachine.h"

#include "BBBeamRevealComponent.generated.h"

class UBBLighthouseBeamComponent;
class AActor;

DECLARE_MULTICAST_DELEGATE(FBBRevealCompleted);

UCLASS(ClassGroup = (BlackBeacon), config = Game, meta = (BlueprintSpawnableComponent))
class UBBBeamRevealComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBBBeamRevealComponent();

	// Called by the beam every tick it drives (see UBBLighthouseBeamComponent).
	void UpdateFromBeam(const BlackBeacon::Logics::FBBBeamQuery& Beam, float DeltaTime);

	// --- state ---
	UFUNCTION(BlueprintCallable, Category = "BlackBeacon|Reveal")
	bool WasFullyRevealed() const { return Machine.WasFullyRevealed(); }

	UFUNCTION(BlueprintCallable, Category = "BlackBeacon|Reveal")
	float GetVisibilityAmount() const { return Machine.GetVisibilityAmount(); }

	BlackBeacon::Logics::EBBRevealPhase GetPhase() const { return Machine.GetPhase(); }

	// Fired the moment the object first becomes fully visible (one-shot
	// discovery beat - objectives/audio/story hooks bind here).
	FBBRevealCompleted OnFullyRevealed;

	// --- per-instance config (ini defaults set the constructor values) ---
	UPROPERTY(config, EditAnywhere, Category = "BlackBeacon|Reveal")
	float RevealDelay = 2.5f;

	UPROPERTY(config, EditAnywhere, Category = "BlackBeacon|Reveal")
	float FadeTime = 1.5f;

	UPROPERTY(config, EditAnywhere, Category = "BlackBeacon|Reveal")
	float MinBeamIntensity = 0.25f;

	UPROPERTY(config, EditAnywhere, Category = "BlackBeacon|Reveal")
	float VisibilityDuration = 0.0f;

	UPROPERTY(config, EditAnywhere, Category = "BlackBeacon|Reveal")
	bool bPersistent = true;

	// Find and subscribe to the world's lighthouse beam in BeginPlay.
	UPROPERTY(EditAnywhere, Category = "BlackBeacon|Reveal")
	bool bAutoSubscribeToBeam = true;

	// One-shot objective completion on first full reveal.
	UPROPERTY(EditAnywhere, Category = "BlackBeacon|Reveal")
	bool bTriggerObjective = true;

	UPROPERTY(EditAnywhere, Category = "BlackBeacon|Reveal")
	FName ObjectiveId = TEXT("BB_OBJ_DISCOVER_ANOMALY");

	// Optional material scalar parameter this component fades (falls back to
	// a straight hidden/visible toggle when NAME_None).
	UPROPERTY(EditAnywhere, Category = "BlackBeacon|Reveal")
	FName FadeMaterialParameterName = NAME_None;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void LocateBeam();
	void HandleFirstFullReveal();
	void ApplyVisibility(float Amount);

	BlackBeacon::Logics::FBBRevealMachine Machine;
	TWeakObjectPtr<UBBLighthouseBeamComponent> Beam = nullptr;
	bool bCompletedCallbackFired = false;

	// Bounded retries when the beam spawns after this component (dev races).
	static constexpr int32 MaxAutoSubscribeAttempts = 4;
	int32 AutoSubscribeAttempts = 0;

	UPROPERTY()
	TObjectPtr<class UStaticMeshComponent> FadeMesh = nullptr;
};