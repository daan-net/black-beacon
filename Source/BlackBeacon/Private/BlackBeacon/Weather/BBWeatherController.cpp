#include "BlackBeacon/Weather/BBWeatherController.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "UObject/ConstructorHelpers.h"

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

	
	RainRoot = CreateDefaultSubobject<USceneComponent>(TEXT("RainRoot"));
	RainRoot->SetupAttachment(RootComponent);

	RainComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("RainComponent")); // Keep for legacy
	RainComponent->SetupAttachment(RainRoot);
	RainComponent->SetVisibility(false);
	RainComponent->bAutoActivate = false;

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> RainAsset(TEXT("/Game/BlackBeacon/Effects/NS_Rain.FountainLightweight"));
	
	// A fixed world-space field prevents the rain from following the player.
	const float EmitterSpacing = RainFieldRadiusCm / 5.0f;
	FRandomStream RainLayoutRandom(0xBB2026);
	for (int32 X = -5; X <= 5; ++X)
	{
		for (int32 Y = -5; Y <= 5; ++Y)
		{
			FString CompName = FString::Printf(TEXT("RainGrid_%d_%d"), X, Y);
			UNiagaraComponent* GridComp = CreateDefaultSubobject<UNiagaraComponent>(*CompName);
			GridComp->SetupAttachment(RainRoot);
			const FVector EmitterOffset(
				X * EmitterSpacing + RainLayoutRandom.FRandRange(-EmitterSpacing * 0.3f, EmitterSpacing * 0.3f),
				Y * EmitterSpacing + RainLayoutRandom.FRandRange(-EmitterSpacing * 0.3f, EmitterSpacing * 0.3f),
				RainLayoutRandom.FRandRange(-150.0f, 150.0f));
			GridComp->SetRelativeLocation(EmitterOffset);
			GridComp->SetRelativeRotation(FRotator(180.0f, 0.0f, 0.0f)); // Point straight down
			// FountainLightweight uses broad sprites, so keep the streaks narrow.
			GridComp->SetRelativeScale3D(FVector(0.08f, 0.08f, 4.0f));
			GridComp->bAutoActivate = false;
			GridComp->SetCastShadow(false);
			
			if (RainAsset.Succeeded())
			{
				GridComp->SetAsset(RainAsset.Object);
			}
			RainGrid.Add(GridComp);
		}
	}
MoonLight->bAtmosphereSunLight = true;
}

void ABBWeatherController::BeginPlay()
{
	Super::BeginPlay();
	MoonLight->SetIntensity(MoonlightLux);
	BuildInterpolatorPalette();
	if (RainRoot)
	{
		RainRoot->SetWorldLocation(GetActorLocation() + FVector(0.0f, 0.0f, RainLayerHeightCm));
	}
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

#include "Engine/World.h"
#include "Engine/HitResult.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

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
	float RainIntensity = static_cast<float>(Interpolator.GetRainIntensity());
	float WindStrength = static_cast<float>(Interpolator.GetWindStrength());

	if (RainRoot && GetWorld())
	{
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (PC)
		{
			// The precipitation field is anchored to the level, independent of view movement.
			FVector CamLoc = PC->PlayerCameraManager ? PC->PlayerCameraManager->GetCameraLocation() : (PC->GetPawn() ? PC->GetPawn()->GetActorLocation() : FVector::ZeroVector);

			// Roof visibility changes slowly compared with camera motion. One
			// query at 4 Hz replaces a line trace per emitter per rendered frame.
			RainOutputUpdateCountdown -= GetWorld()->GetDeltaSeconds();
			if (RainOutputUpdateCountdown > 0.0f)
			{
				return;
			}
			RainOutputUpdateCountdown = 0.25f;

			FCollisionQueryParams Params(SCENE_QUERY_STAT(WeatherIndoorTrace), false);
			if (PC->GetPawn()) Params.AddIgnoredActor(PC->GetPawn());
			if (PC->GetViewTarget()) Params.AddIgnoredActor(PC->GetViewTarget());

			FHitResult Hit;
			const FVector RoofProbeEnd = CamLoc + FVector(0.0f, 0.0f, 6000.0f);
			const bool bUnderRoof = GetWorld()->LineTraceSingleByChannel(
				Hit, CamLoc, RoofProbeEnd, ECC_WorldStatic, Params);
			const bool bShouldRain = RainIntensity > 0.05f && !bUnderRoof;
			if (bShouldRain != bRainGridActive)
			{
				for (UNiagaraComponent* GridComp : RainGrid)
				{
					if (!GridComp) continue;
					if (bShouldRain) GridComp->Activate(true);
					else GridComp->Deactivate();
				}
				bRainGridActive = bShouldRain;
			}

			if (bRainGridActive)
			{
				for (UNiagaraComponent* GridComp : RainGrid)
				{
					if (GridComp)
					{
						GridComp->SetFloatParameter(TEXT("RainIntensity"), RainIntensity);
						GridComp->SetFloatParameter(TEXT("WindStrength"), WindStrength);
					}
				}
			}
		}
	}
}
