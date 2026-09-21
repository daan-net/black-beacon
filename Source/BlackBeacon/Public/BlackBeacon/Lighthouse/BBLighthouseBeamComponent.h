// BLACK BEACON - lighthouse beam component. [SIGNATURE SYSTEM]
//
// The reusable heart of the game. Owns:
//   * rotation state machine (Off / Auto sweep / Manual player aim)
//   * intensity (target/current lerp + flicker)
//   * range / cone width
//   * power gating (dead lantern when unpowered)
//   * a visible spotlight driving volumetric fog contribution
//   * the world-facing beam query that reveal systems consume
//
// Reveal objects subscribe explicitly; the beam updates them ONLY when its
// state changes (event-driven, no per-frame world scans). All gameplay
// tunables are config UPROPERTYs (Config/DefaultGame.ini).

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"

#include "BlackBeacon/Logics/BBBeamMath.h"

#include "BBLighthouseBeamComponent.generated.h"

class USpotLightComponent;
class UBBBeamRevealComponent;

UENUM(BlueprintType)
enum class EBBBeamRotationMode : uint8
{
	Off,
	Auto,
	Manual
};

DECLARE_MULTICAST_DELEGATE_OneParam(FBBBeamQueryChanged, const BlackBeacon::Logics::FBBBeamQuery& /*Query*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FBBBeamRotationModeChanged, EBBBeamRotationMode /*Mode*/);

UCLASS(ClassGroup = (BlackBeacon), config = Game, meta = (BlueprintSpawnableComponent))
class UBBLighthouseBeamComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UBBLighthouseBeamComponent();

	// --- control ---
	UFUNCTION(BlueprintCallable, Category = "BlackBeacon|Beam")
	void SetRotationMode(EBBBeamRotationMode InMode);

	UFUNCTION(BlueprintCallable, Category = "BlackBeacon|Beam")
	void SetManualYawTarget(float YawDegrees);

	UFUNCTION(BlueprintCallable, Category = "BlackBeacon|Beam")
	void SetManualPitchDegrees(float PitchDegrees);

	UFUNCTION(BlueprintCallable, Category = "BlackBeacon|Beam")
	void SetPowered(bool bInPowered);

	UFUNCTION(BlueprintCallable, Category = "BlackBeacon|Beam")
	void SetBeamIntensityTarget(float Intensity01);

	// --- queries ---
	BlackBeacon::Logics::FBBBeamQuery GetBeamQuery() const;

	UFUNCTION(BlueprintCallable, Category = "BlackBeacon|Beam")
	float GetCurrentYawDegrees() const { return CurrentYawDeg; }

	UFUNCTION(BlueprintCallable, Category = "BlackBeacon|Beam")
	EBBBeamRotationMode GetRotationMode() const { return RotationMode; }

	bool IsPowered() const { return bPowered; }

	// --- reveal subscriptions (called by reveal components) ---
	void SubscribeRevealComponent(UBBBeamRevealComponent* Reveal);
	void UnsubscribeRevealComponent(UBBBeamRevealComponent* Reveal);

	// --- events ---
	FBBBeamQueryChanged OnBeamQueryChanged;
	FBBBeamRotationModeChanged OnRotationModeChanged;

	// --- config ---
	UPROPERTY(config, EditAnywhere, Category = "BlackBeacon|Beam")
	float AutoRotationDegPerSec = 6.0f;

	UPROPERTY(config, EditAnywhere, Category = "BlackBeacon|Beam")
	float ManualRotationDegPerSec = 60.0f;

	UPROPERTY(config, EditAnywhere, Category = "BlackBeacon|Beam")
	float BeamRangeCm = 180000.0f;

	UPROPERTY(config, EditAnywhere, Category = "BlackBeacon|Beam")
	float BeamHalfAngleDeg = 6.0f;

	UPROPERTY(config, EditAnywhere, Category = "BlackBeacon|Beam")
	float BeamMaxIntensityLumens = 8000000.0f;

	UPROPERTY(config, EditAnywhere, Category = "BlackBeacon|Beam")
	FLinearColor BeamColor = FLinearColor(1.0f, 0.92f, 0.78f, 1.0f);

	UPROPERTY(config, EditAnywhere, Category = "BlackBeacon|Beam")
	bool bStartInAutoRotation = true;

	UPROPERTY(config, EditAnywhere, Category = "BlackBeacon|Beam")
	bool bVolumetricLight = true;

	// >0: simulates a periodic mechanical flicker (frequency refills per second).
	UPROPERTY(EditAnywhere, Category = "BlackBeacon|Beam")
	float FlickerFrequency = 0.0f;

	UPROPERTY(EditAnywhere, Category = "BlackBeacon|Beam")
	float FlickerAmplitude = 0.25f;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	// Visible representation: the spotlight that reads as a volume in fog.
	UPROPERTY(VisibleAnywhere, Category = "BlackBeacon|Beam")
	TObjectPtr<USpotLightComponent> BeamLight = nullptr;

private:
	bool UpdateRotation(float DeltaTime);
	bool UpdateIntensity(float DeltaTime);
	void UpdateReveals(const BlackBeacon::Logics::FBBBeamQuery& Query, float DeltaTime);
	void PublishBeamState();

	BlackBeacon::Logics::FBBBeamQuery BuildQuery() const;
	void ApplyVisibleState();

	EBBBeamRotationMode RotationMode = EBBBeamRotationMode::Off;
	float CurrentYawDeg = 0.0f;
	float TargetYawDeg = 0.0f;
	float CurrentPitchDeg = 0.0f;

	float IntensityTarget = 1.0f;
	float IntensityCurrent = 1.0f;
	float FlickerPhase = 0.0f;
	bool bPowered = false;

	TArray<TWeakObjectPtr<UBBBeamRevealComponent>> SubscribedReveals;
};
