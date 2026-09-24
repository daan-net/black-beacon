// BLACK BEACON - weather controller.
//
// Owns the slice's atmosphere: a state machine (Clear/Fog/Rain/Storm) with
// timed interpolation driven by the engine-free FBBWeatherInterpolator.
// Drives exponential height fog, sky lighting, and a bounded world-space rain
// field from the engine-independent weather interpolation state.
//
// Only one of these should exist in a level. The greybox bootstrap spawns
// it; authored maps replace it with a placed instance.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "BlackBeacon/Logics/BBWeatherState.h"

#include "BBWeatherController.generated.h"

class UVolumetricCloudComponent;
class UBBStormPresentationComponent;
class UExponentialHeightFogComponent;
class UDirectionalLightComponent;
class USkyAtmosphereComponent;
class USkyLightComponent;
class UInstancedStaticMeshComponent;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;

// UE-facing mirror of the logic-layer phase enum (config-friendly).
UENUM(BlueprintType)
enum class EBBWeatherPhase : uint8
{
	Clear,
	Fog,
	Rain,
	Storm
};

// UE-facing palette entry mirroring BlackBeacon::Logics::FBBWeatherPaletteEntry.
USTRUCT(BlueprintType)
struct FBBWeatherPaletteConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "BlackBeacon|Weather")
	EBBWeatherPhase Phase = EBBWeatherPhase::Clear;

	UPROPERTY(EditAnywhere, Category = "BlackBeacon|Weather")
	float FogDensity = 0.0008f;

	UPROPERTY(EditAnywhere, Category = "BlackBeacon|Weather")
	float WindStrength = 0.0f;

	UPROPERTY(EditAnywhere, Category = "BlackBeacon|Weather")
	float RainIntensity = 0.0f;

	UPROPERTY(EditAnywhere, Category = "BlackBeacon|Weather")
	float Cloudiness = 0.15f;

	UPROPERTY(EditAnywhere, Category = "BlackBeacon|Weather")
	FLinearColor FogColor = FLinearColor(0.30f, 0.32f, 0.35f, 1.0f);
};

UCLASS(config = Game)
class ABBWeatherController : public AActor
{
	GENERATED_BODY()

public:
	ABBWeatherController();

	UFUNCTION(BlueprintCallable, Category = "BlackBeacon|Weather")
	void SetWeather(EBBWeatherPhase Phase, float InTransitionSeconds = -1.0f);

	UFUNCTION(BlueprintCallable, Category = "BlackBeacon|Weather")
	EBBWeatherPhase GetTargetPhase() const;

	UFUNCTION(BlueprintCallable, Category = "BlackBeacon|Weather")
	float GetFogDensity() const;

	// The beam/reveal systems read this to know how readable light is now.
	UFUNCTION(BlueprintCallable, Category = "BlackBeacon|Weather")
	float GetFogDensityMultiplier() const;

	UPROPERTY(VisibleAnywhere, Category = "BlackBeacon|Weather")
	TObjectPtr<UExponentialHeightFogComponent> FogComponent = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "BlackBeacon|Weather")
	TObjectPtr<UDirectionalLightComponent> MoonLight = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "BlackBeacon|Weather")
	TObjectPtr<USkyAtmosphereComponent> SkyAtmosphere = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "BlackBeacon|Weather")
	TObjectPtr<USkyLightComponent> SkyLight = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "BlackBeacon|Weather")
	TObjectPtr<UStaticMeshComponent> SkyCloudDome = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "BlackBeacon|Weather")
	TObjectPtr<USceneComponent> RainRoot = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "BlackBeacon|Weather")
	TObjectPtr<UInstancedStaticMeshComponent> RainField = nullptr;

    UPROPERTY(VisibleAnywhere, Category = "BlackBeacon|Weather")
    TObjectPtr<UVolumetricCloudComponent> StormClouds = nullptr;

    UPROPERTY(VisibleAnywhere, Category = "BlackBeacon|Weather")
    TObjectPtr<UBBStormPresentationComponent> StormPresentation = nullptr;

    FVector GetWindVelocity() const;
    float GetWindStrength() const { return static_cast<float>(Interpolator.GetWindStrength()); }
    float GetRainIntensity() const { return static_cast<float>(Interpolator.GetRainIntensity()); }
    bool IsListenerSheltered() const { return bListenerSheltered; }

    UPROPERTY(config, EditAnywhere, Category = "BlackBeacon|Weather")
    float StormWindYawDegrees = 22.0f;

	// --- config ---
	UPROPERTY(config, EditAnywhere, Category = "BlackBeacon|Weather")
	EBBWeatherPhase InitialPhase = EBBWeatherPhase::Rain;

	UPROPERTY(config, EditAnywhere, Category = "BlackBeacon|Weather")
	float TransitionSeconds = 8.0f;

	UPROPERTY(config, EditAnywhere, Category = "BlackBeacon|Weather")
	float MoonlightLux = 1.5f;

	UPROPERTY(config, EditAnywhere, Category = "BlackBeacon|Weather")
	float MoonSkyFillIntensity = 0.1f;

	UPROPERTY(config, EditAnywhere, Category = "BlackBeacon|Weather|Sky")
	float SkyCloudOpacity = 0.8f;

	UPROPERTY(config, EditAnywhere, Category = "BlackBeacon|Weather|Rain")
	float RainFieldRadiusCm = 5500.0f;

	UPROPERTY(config, EditAnywhere, Category = "BlackBeacon|Weather|Rain")
	int32 RainParticleCount = 7000;

	UPROPERTY(config, EditAnywhere, Category = "BlackBeacon|Weather|Rain")
	float RainVolumeHeightCm = 6000.0f;

	UPROPERTY(config, EditAnywhere, Category = "BlackBeacon|Weather|Rain")
	float RainFallSpeedCmPerSecond = 2800.0f;

	UPROPERTY(config, EditAnywhere, Category = "BlackBeacon|Weather|Rain")
	float RainWindDriftCmPerSecond = 650.0f;

	UPROPERTY(config, EditAnywhere, Category = "BlackBeacon|Weather|Rain")
	float RainStreakWidthCm = 1.8f;

	UPROPERTY(config, EditAnywhere, Category = "BlackBeacon|Weather|Rain")
	float RainStreakMinLengthCm = 50.0f;

	UPROPERTY(config, EditAnywhere, Category = "BlackBeacon|Weather|Rain")
	float RainStreakMaxLengthCm = 100.0f;

	// Palette overrides; leave phases out to keep the struct defaults.
	UPROPERTY(config, EditAnywhere, Category = "BlackBeacon|Weather")
	TArray<FBBWeatherPaletteConfig> Palette;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	void BuildInterpolatorPalette();
	void ApplyToFog();
	void ApplyOutputs();
	void InitializeRainField();
	void UpdateRainField(float DeltaSeconds);
	void RespawnRainParticle(int32 ParticleIndex);
	FTransform BuildRainTransform(int32 ParticleIndex, bool bVisible) const;

	struct FRainParticleState
	{
		FVector Position = FVector::ZeroVector;
		FVector LateralDrift = FVector::ZeroVector;
		float FallSpeed = 0.0f;
		float LengthCm = 0.0f;
		float WidthCm = 0.0f;
		float PlaneRollRadians = 0.0f;
	};

	BlackBeacon::Logics::FBBWeatherInterpolator Interpolator;
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> SkyCloudMaterial = nullptr;
	TArray<FRainParticleState> RainParticles;
	TArray<FTransform> RainInstanceTransforms;
	FRandomStream RainRandomStream;
	float FogDensityBase = 1.0f; // reserved: authored-map fog scaling
	float RainOutputUpdateCountdown = 0.0f;
	bool bRainFieldActive = false;
    bool bListenerSheltered = false;
    FVector CurrentWindVelocity = FVector::ZeroVector;
	int32 ActiveRainParticleCount = INDEX_NONE;
};
