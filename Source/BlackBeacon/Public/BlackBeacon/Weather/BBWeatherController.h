// BLACK BEACON - weather controller.
//
// Owns the slice's atmosphere: a state machine (Clear/Fog/Rain/Storm) with
// timed interpolation driven by the engine-free FBBWeatherInterpolator.
// Gadgets: exponential height fog (density/colour) and scalar outputs wind,
// rain, cloudiness for later particle/vegetation systems.
//
// Only one of these should exist in a level. The greybox bootstrap spawns
// it; authored maps replace it with a placed instance.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "BlackBeacon/Logics/BBWeatherState.h"

#include "BBWeatherController.generated.h"

class UExponentialHeightFogComponent;
class UDirectionalLightComponent;
class USkyAtmosphereComponent;
class USkyLightComponent;
class UNiagaraComponent;

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
	TObjectPtr<UNiagaraComponent> RainComponent = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "BlackBeacon|Weather")
	TObjectPtr<USceneComponent> RainRoot = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "BlackBeacon|Weather")
	TArray<TObjectPtr<UNiagaraComponent>> RainGrid;

	// --- config ---
	UPROPERTY(config, EditAnywhere, Category = "BlackBeacon|Weather")
	EBBWeatherPhase InitialPhase = EBBWeatherPhase::Rain;

	UPROPERTY(config, EditAnywhere, Category = "BlackBeacon|Weather")
	float TransitionSeconds = 8.0f;

	UPROPERTY(config, EditAnywhere, Category = "BlackBeacon|Weather")
	float MoonlightLux = 1.5f;

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

	BlackBeacon::Logics::FBBWeatherInterpolator Interpolator;
	float FogDensityBase = 1.0f; // reserved: authored-map fog scaling
};
