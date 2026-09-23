#include "BlackBeacon/Weather/BBWeatherController.h"

#include "Components/ExponentialHeightFogComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"

ABBWeatherController::ABBWeatherController()
{
	PrimaryActorTick.bCanEverTick = true; // single instance; cheap lerp driving
	PrimaryActorTick.TickGroup = TG_PrePhysics;

	FogComponent = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("FogComponent"));
	RootComponent = FogComponent;
	FogComponent->SetVolumetricFog(true);
	FogComponent->SetVolumetricFogScatteringDistribution(0.0f);
	FogComponent->SetVolumetricFogDistance(10000.0f);

	MoonLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("MoonLight"));
	MoonLight->SetupAttachment(FogComponent);
	MoonLight->SetMobility(EComponentMobility::Movable);
	MoonLight->SetRelativeRotation(FRotator(-35.0f, 35.0f, 0.0f));
	MoonLight->SetLightColor(FLinearColor(0.32f, 0.48f, 0.75f));
	MoonLight->SetCastShadows(false);

	SkyAtmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("SkyAtmosphere"));
	SkyAtmosphere->SetupAttachment(FogComponent);
	
	SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
	SkyLight->SetupAttachment(FogComponent);
	SkyLight->bRealTimeCapture = true;
	SkyLight->SetIntensity(0.1f);

	MoonLight->bAtmosphereSunLight = true;
}

void ABBWeatherController::BeginPlay()
{
	Super::BeginPlay();
	MoonLight->SetIntensity(MoonlightLux);
	BuildInterpolatorPalette();
	SetWeather(InitialPhase, /*InTransitionSeconds=*/0.0f);
	ApplyToFog();
}

void ABBWeatherController::BuildInterpolatorPalette()
{
	// Start from a full default palette, then apply configured overrides.
	std::vector<BlackBeacon::Logics::FBBWeatherPaletteEntry> Entries;
	for (std::uint8_t I = 0; I < static_cast<std::uint8_t>(BlackBeacon::Logics::EBBWeatherPhase::Count); ++I)
	{
		BlackBeacon::Logics::FBBWeatherPaletteEntry Entry;
		Entry.Phase = static_cast<BlackBeacon::Logics::EBBWeatherPhase>(I);
		Entries.push_back(Entry);
	}

	for (const FBBWeatherPaletteConfig& Config : Palette)
	{
		const std::uint8_t Index = static_cast<std::uint8_t>(Config.Phase);
		if (Index >= Entries.size())
		{
			continue;
		}
		BlackBeacon::Logics::FBBWeatherPaletteEntry& Entry = Entries[Index];
		Entry.Phase = static_cast<BlackBeacon::Logics::EBBWeatherPhase>(Index);
		Entry.FogDensity = Config.FogDensity;
		Entry.WindStrength = Config.WindStrength;
		Entry.RainIntensity = Config.RainIntensity;
		Entry.Cloudiness = Config.Cloudiness;
		Entry.FogR = Config.FogColor.R;
		Entry.FogG = Config.FogColor.G;
		Entry.FogB = Config.FogColor.B;
	}

	Interpolator.Configure(Entries);
}

void ABBWeatherController::SetWeather(EBBWeatherPhase Phase, float InTransitionSeconds)
{
	const float Seconds = InTransitionSeconds < 0.0f ? TransitionSeconds : InTransitionSeconds;
	Interpolator.SetTarget(static_cast<BlackBeacon::Logics::EBBWeatherPhase>(Phase), Seconds);
}

EBBWeatherPhase ABBWeatherController::GetTargetPhase() const
{
	return static_cast<EBBWeatherPhase>(Interpolator.GetTargetPhase());
}

float ABBWeatherController::GetFogDensity() const
{
	return static_cast<float>(Interpolator.GetFogDensity());
}

float ABBWeatherController::GetFogDensityMultiplier() const
{
	// Higher fog density relative to "clear" = beam cuts less through it.
	// 0.1 baseline: 1.0 at the configured clear density.
	return GetFogDensity() / 0.0008f;
}

void ABBWeatherController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	Interpolator.Tick(DeltaSeconds);
	ApplyToFog();
	ApplyOutputs();
}

void ABBWeatherController::ApplyToFog()
{
	if (!FogComponent)
	{
		return;
	}

	double R = 0.0, G = 0.0, B = 0.0;
	Interpolator.GetFogColor(R, G, B);

	FogComponent->SetFogDensity(static_cast<float>(Interpolator.GetFogDensity()));
	FogComponent->SetFogInscatteringColor(FLinearColor(static_cast<float>(R), static_cast<float>(G), static_cast<float>(B), 1.0f));
}

void ABBWeatherController::ApplyOutputs()
{
	// Wind/rain/cloudiness are exposed for future particle/vegetation
	// systems (Niagara rain in 0.2 binds to GetRainIntensity-equivalents).
	// Nothing to write in 0.1 beyond the fog above.
}
