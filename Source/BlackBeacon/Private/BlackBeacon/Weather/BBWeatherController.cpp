#include "BlackBeacon/Weather/BBWeatherController.h"
#include "UObject/ConstructorHelpers.h"
#include "BlackBeacon/Weather/BBStormPresentationComponent.h"
#include "BlackBeacon/Logics/BBStormTiming.h"
#include "Components/VolumetricCloudComponent.h"

#include "Components/ExponentialHeightFogComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

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

    StormClouds = CreateDefaultSubobject<UVolumetricCloudComponent>(TEXT("StormClouds"));
    StormClouds->SetupAttachment(FogComponent);
    StormPresentation = CreateDefaultSubobject<UBBStormPresentationComponent>(TEXT("StormPresentation"));

	SkyAtmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("SkyAtmosphere"));
	SkyAtmosphere->SetupAttachment(FogComponent);
	
	SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
	SkyLight->SetupAttachment(FogComponent);
	// The sky fill is configured at runtime and captures the moving storm dome.
	SkyLight->SetMobility(EComponentMobility::Movable);
	SkyLight->bRealTimeCapture = true;
	SkyLight->SetLightColor(FLinearColor(0.48f, 0.60f, 0.86f));

	SkyCloudDome = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SkyCloudDome"));
	SkyCloudDome->SetupAttachment(RootComponent);
	SkyCloudDome->SetMobility(EComponentMobility::Movable);
	SkyCloudDome->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SkyCloudDome->SetCastShadow(false);
	SkyCloudDome->SetReceivesDecals(false);
	SkyCloudDome->SetCanEverAffectNavigation(false);
	SkyCloudDome->SetReverseCulling(false);
	SkyCloudDome->SetRelativeScale3D(FVector(40000.0f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SkyDomeMesh(
		TEXT("/Engine/EngineSky/SM_SkySphere.SM_SkySphere"));
	if (SkyDomeMesh.Succeeded())
	{
		SkyCloudDome->SetStaticMesh(SkyDomeMesh.Object);
	}
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> SkyCloudBase(
		TEXT("/Game/BlackBeacon/Materials/M_StormSky.M_StormSky"));
	if (SkyCloudBase.Succeeded())
	{
		SkyCloudDome->SetMaterial(0, SkyCloudBase.Object);
		SkyCloudMaterial = SkyCloudDome->CreateAndSetMaterialInstanceDynamic(0);
	}

	RainRoot = CreateDefaultSubobject<USceneComponent>(TEXT("RainRoot"));
	RainRoot->SetupAttachment(RootComponent);
	RainField = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("RainField"));
	RainField->SetupAttachment(RainRoot);
	RainField->SetMobility(EComponentMobility::Movable);
	RainField->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RainField->SetGenerateOverlapEvents(false);
	RainField->SetCastShadow(false);
	RainField->SetReceivesDecals(false);
	RainField->SetVisibility(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> RainDropMesh(TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (RainDropMesh.Succeeded())
	{
		RainField->SetStaticMesh(RainDropMesh.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> RainDropMaterial(
		TEXT("/Game/BlackBeacon/Materials/M_RainStreak.M_RainStreak"));
	if (RainDropMaterial.Succeeded())
	{
		RainField->SetMaterial(0, RainDropMaterial.Object);
	}
MoonLight->bAtmosphereSunLight = true;
}

void ABBWeatherController::BeginPlay()
{
	Super::BeginPlay();
    LoadConfig();
	MoonLight->SetIntensity(MoonlightLux);
	SkyLight->SetIntensity(MoonSkyFillIntensity);
	BuildInterpolatorPalette();
	if (RainRoot)
	{
		RainRoot->SetWorldLocation(GetActorLocation());
	}
	SetWeather(InitialPhase, /*InTransitionSeconds=*/0.0f);
	InitializeRainField();
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
    const float Gust = static_cast<float>(BlackBeacon::Logics::StormGust(GetWorld()->GetTimeSeconds()));
    CurrentWindVelocity = FRotator(0,StormWindYawDegrees,0).Vector() * RainWindDriftCmPerSecond * GetWindStrength() * Gust;
	ApplyToFog();
	ApplyOutputs();
	UpdateRainField(DeltaSeconds);
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
	const float RainIntensity = static_cast<float>(Interpolator.GetRainIntensity());
	const float Cloudiness = static_cast<float>(Interpolator.GetCloudiness());
	if (SkyCloudMaterial)
	{
		SkyCloudMaterial->SetScalarParameterValue(TEXT("CloudOpacity"), SkyCloudOpacity * Cloudiness);
	}

	if (RainRoot && GetWorld())
	{
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (PC)
		{
			// The precipitation field is anchored to the level, independent of view movement.
			FVector CamLoc = PC->PlayerCameraManager ? PC->PlayerCameraManager->GetCameraLocation() : (PC->GetPawn() ? PC->GetPawn()->GetActorLocation() : FVector::ZeroVector);
			if (SkyCloudDome)
			{
				SkyCloudDome->SetWorldLocation(CamLoc);
			}

			// A single roof probe is enough to cull the local rain field indoors.
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
			bListenerSheltered = bUnderRoof;
            const bool bShouldRain = RainIntensity > 0.05f && !bUnderRoof;
			if (bShouldRain != bRainFieldActive)
			{
				RainField->SetVisibility(bShouldRain, true);
				bRainFieldActive = bShouldRain;
			}
		}
	}
}

void ABBWeatherController::InitializeRainField()
{
	if (!RainField || RainParticleCount <= 0)
	{
		return;
	}

	RainRandomStream.Initialize(0xBB2026);
	RainParticles.SetNum(RainParticleCount);
	RainInstanceTransforms.SetNum(RainParticleCount);
	RainField->ClearInstances();

	for (int32 ParticleIndex = 0; ParticleIndex < RainParticleCount; ++ParticleIndex)
	{
		RespawnRainParticle(ParticleIndex);
		const FTransform Transform = BuildRainTransform(ParticleIndex, true);
		RainInstanceTransforms[ParticleIndex] = Transform;
		RainField->AddInstance(Transform, false);
	}
	ActiveRainParticleCount = RainParticleCount;
}

void ABBWeatherController::RespawnRainParticle(int32 ParticleIndex)
{
	if (!RainParticles.IsValidIndex(ParticleIndex))
	{
		return;
	}

	FRainParticleState& Particle = RainParticles[ParticleIndex];
	Particle.Position = FVector(
		RainRandomStream.FRandRange(-RainFieldRadiusCm, RainFieldRadiusCm),
		RainRandomStream.FRandRange(-RainFieldRadiusCm, RainFieldRadiusCm),
		RainRandomStream.FRandRange(0.0f, RainVolumeHeightCm));
	Particle.LateralDrift = FVector(
		RainRandomStream.FRandRange(-120.0f, 120.0f),
		RainRandomStream.FRandRange(-90.0f, 90.0f),
		0.0f);
	Particle.FallSpeed = RainFallSpeedCmPerSecond * RainRandomStream.FRandRange(0.78f, 1.22f);
	Particle.LengthCm = RainRandomStream.FRandRange(RainStreakMinLengthCm, RainStreakMaxLengthCm);
	Particle.WidthCm = RainStreakWidthCm * RainRandomStream.FRandRange(0.7f, 1.3f);
	Particle.PlaneRollRadians = RainRandomStream.FRandRange(0.0f, UE_TWO_PI);
}

FTransform ABBWeatherController::BuildRainTransform(int32 ParticleIndex, bool bVisible) const
{
	const FRainParticleState& Particle = RainParticles[ParticleIndex];
	const FVector Velocity = Particle.LateralDrift + GetWindVelocity() + FVector(0.0f, 0.0f, -Particle.FallSpeed);
	const FVector FallDirection = Velocity.GetSafeNormal();
	const FQuat AlignLengthWithFall = FQuat::FindBetweenNormals(FVector::YAxisVector, FallDirection);
	const FQuat RollAroundFall = FQuat(FVector::YAxisVector, Particle.PlaneRollRadians);
	const FQuat Orientation = AlignLengthWithFall * RollAroundFall;
	const FVector Scale = bVisible
		? FVector(Particle.WidthCm / 100.0f, Particle.LengthCm / 100.0f, 1.0f)
		: FVector::ZeroVector;
	return FTransform(Orientation, Particle.Position, Scale);
}

void ABBWeatherController::UpdateRainField(float DeltaSeconds)
{
	if (!RainField || RainParticles.IsEmpty() || RainInstanceTransforms.IsEmpty())
	{
		return;
	}

	const float RainIntensity = static_cast<float>(Interpolator.GetRainIntensity());
	const int32 NewActiveCount = FMath::Clamp(
		FMath::RoundToInt(static_cast<float>(RainParticleCount) * RainIntensity), 0, RainParticles.Num());
	ActiveRainParticleCount = NewActiveCount;

	for (int32 ParticleIndex = 0; ParticleIndex < RainParticles.Num(); ++ParticleIndex)
	{
		FRainParticleState& Particle = RainParticles[ParticleIndex];
		Particle.Position += (Particle.LateralDrift + GetWindVelocity()
            + FVector(0,0,-Particle.FallSpeed)) * DeltaSeconds;

		if (Particle.Position.Z < 0.0f || FMath::Abs(Particle.Position.X)>RainFieldRadiusCm
            || FMath::Abs(Particle.Position.Y)>RainFieldRadiusCm)
		{
			RespawnRainParticle(ParticleIndex);
		}

		RainInstanceTransforms[ParticleIndex] = BuildRainTransform(
			ParticleIndex, ParticleIndex < ActiveRainParticleCount);
	}

	// One instanced mesh batches thousands of independent, world-space streaks
	// into one draw component instead of a grid of point-source Niagara systems.
	RainField->BatchUpdateInstancesTransforms(
		0, RainInstanceTransforms, false, true, true);
}

FVector ABBWeatherController::GetWindVelocity() const
{
    return CurrentWindVelocity;
}
