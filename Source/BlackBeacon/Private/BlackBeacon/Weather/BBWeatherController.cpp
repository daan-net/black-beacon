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
	
	// Create a 3x3 grid of emitters to cover a wide area without a visible single origin
	for (int32 X = -1; X <= 1; ++X)
	{
		for (int32 Y = -1; Y <= 1; ++Y)
		{
			FString CompName = FString::Printf(TEXT("RainGrid_%d_%d"), X, Y);
			UNiagaraComponent* GridComp = CreateDefaultSubobject<UNiagaraComponent>(*CompName);
			GridComp->SetupAttachment(RainRoot);
			GridComp->SetRelativeLocation(FVector(X * 1200.0f, Y * 1200.0f, 0.0f));
			GridComp->SetRelativeRotation(FRotator(180.0f, 0.0f, 0.0f)); // Upside down so fountain shoots down
			GridComp->SetRelativeScale3D(FVector(4.0f, 4.0f, 1.0f));
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

#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Engine/HitResult.h"

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

	bool bIsIndoors = false;
	if (RainRoot && GetWorld())
	{
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (PC && PC->GetPawn())
		{
			FVector PlayerLoc = PC->GetPawn()->GetActorLocation();
			RainRoot->SetWorldLocation(FVector(PlayerLoc.X, PlayerLoc.Y, PlayerLoc.Z + 2500.0f));

			// Raycast up to see if under a roof
			FHitResult Hit;
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(PC->GetPawn());
			Params.AddIgnoredActor(this);
			
			if (GetWorld()->LineTraceSingleByChannel(Hit, PlayerLoc, PlayerLoc + FVector(0, 0, 10000.0f), ECC_Visibility, Params))
			{
				bIsIndoors = true;
			}
		}
	}

	if (bIsIndoors)
	{
		RainIntensity = 0.0f;
	}

	for (UNiagaraComponent* GridComp : RainGrid)
	{
		if (!GridComp) continue;

		GridComp->SetFloatParameter(TEXT("RainIntensity"), RainIntensity);
		GridComp->SetFloatParameter(TEXT("WindStrength"), WindStrength);

		if (RainIntensity > 0.01f)
		{
			if (!GridComp->IsActive()) GridComp->Activate(true);
			// Fountain specific logic to spread and scale
			GridComp->SetFloatParameter(TEXT("SpawnRate"), RainIntensity * 2000.0f); 
		}
		else
		{
			if (GridComp->IsActive()) GridComp->Deactivate();
		}
	}
}
