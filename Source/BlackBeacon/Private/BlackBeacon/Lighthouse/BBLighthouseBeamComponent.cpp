#include "BlackBeacon/Lighthouse/BBLighthouseBeamComponent.h"

#include "Components/LightComponentBase.h"
#include "Components/SpotLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

#include "BlackBeacon/Lighthouse/BBBeamRevealComponent.h"

UBBLighthouseBeamComponent::UBBLighthouseBeamComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;

	BeamVisualPivot = CreateDefaultSubobject<USceneComponent>(TEXT("BeamVisualPivot"));
	BeamVisualPivot->SetupAttachment(this);

	BeamLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("BeamLight"));
	BeamLight->SetupAttachment(BeamVisualPivot);
	BeamLight->SetMobility(EComponentMobility::Movable);
	BeamLight->SetCastShadows(true);
	BeamOriginGlow = CreateDefaultSubobject<UPointLightComponent>(TEXT("BeamOriginGlow"));
	BeamOriginGlow->SetupAttachment(this);
	BeamOriginGlow->SetCastShadows(false);
	BeamLensMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BeamLensMesh"));
	BeamLensMesh->SetupAttachment(this);
	BeamLensMesh->SetMobility(EComponentMobility::Movable);
	BeamLensMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BeamLensMesh->SetCastShadow(false);
	BeamLensMesh->SetRelativeScale3D(FVector(0.8f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> LensSphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> LensMaterial(TEXT("/Game/BlackBeacon/Materials/M_LanternLens.M_LanternLens"));
	if (LensSphere.Succeeded())
	{
		BeamLensMesh->SetStaticMesh(LensSphere.Object);
	}
	if (LensMaterial.Succeeded())
	{
		BeamLensMesh->SetMaterial(0, LensMaterial.Object);
	}
	// The runtime greybox has no authored lantern housing yet; a narrow mast
	// grounds the light source on the tower without affecting traversal.
	BeamLensSupport = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BeamLensSupport"));
	BeamLensSupport->SetupAttachment(this);
	BeamLensSupport->SetMobility(EComponentMobility::Movable);
	BeamLensSupport->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BeamLensSupport->SetCastShadow(false);
	BeamLensSupport->SetRelativeLocation(FVector(0.0f, 0.0f, -210.0f));
	BeamLensSupport->SetRelativeScale3D(FVector(0.5f, 0.5f, 4.2f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SupportCylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (SupportCylinder.Succeeded())
	{
		BeamLensSupport->SetStaticMesh(SupportCylinder.Object);
	}

	BeamVisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BeamVisualMesh"));
	BeamVisualMesh->SetupAttachment(BeamVisualPivot);
	BeamVisualMesh->SetMobility(EComponentMobility::Movable);
	BeamVisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BeamVisualMesh->SetCastShadow(false);
	BeamVisualMesh->SetReceivesDecals(false);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cone(TEXT("/Engine/BasicShapes/Cone.Cone"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> VolumeMaterial(TEXT("/Game/BlackBeacon/Materials/M_BeamShaft.M_BeamShaft"));
	if (Cone.Succeeded())
	{
		BeamVisualMesh->SetStaticMesh(Cone.Object);
	}
	if (VolumeMaterial.Succeeded())
	{
		BeamVisualMesh->SetMaterial(0, VolumeMaterial.Object);
	}
}

void UBBLighthouseBeamComponent::BeginPlay()
{
	Super::BeginPlay();

	if (BeamLight)
	{
		BeamLight->IntensityUnits = ELightUnits::Lumens;
		BeamLight->SetIntensity(BeamMaxIntensityLumens);
		BeamLight->SetLightColor(BeamColor);
		BeamLight->SetAttenuationRadius(BeamRangeCm);
		BeamLight->SetOuterConeAngle(BeamHalfAngleDeg);
		BeamLight->SetInnerConeAngle(BeamHalfAngleDeg * 0.5f);
		BeamLight->SetVolumetricScatteringIntensity(bVolumetricLight ? BeamVolumetricScatteringIntensity : 0.0f);
	}
	if (BeamOriginGlow)
	{
		BeamOriginGlow->IntensityUnits = ELightUnits::Lumens;
		BeamOriginGlow->SetIntensity(BeamOriginGlowLumens);
		BeamOriginGlow->SetLightColor(BeamColor);
		BeamOriginGlow->SetAttenuationRadius(1800.0f);
		BeamOriginGlow->SetVolumetricScatteringIntensity(0.5f);
	}
	if (BeamVisualMesh)
	{
		const float VisualLength = FMath::Max(BeamVisualLengthCm, 100.0f);
		const float EndRadius = FMath::Tan(FMath::DegreesToRadians(BeamHalfAngleDeg)) * VisualLength;
		BeamVisualMesh->SetRelativeLocation(FVector(VisualLength * 0.5f, 0.0f, 0.0f));
		BeamVisualMesh->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
		BeamVisualMesh->SetRelativeScale3D(FVector(EndRadius / 50.0f, EndRadius / 50.0f, VisualLength / 100.0f));
		BeamVisualMaterial = BeamVisualMesh->CreateDynamicMaterialInstance(0);
		if (BeamVisualMaterial)
		{
			BeamVisualMaterial->SetScalarParameterValue(TEXT("BeamOpacity"), BeamVisualOpacity);
			BeamVisualMaterial->SetScalarParameterValue(TEXT("VisualLengthCm"), VisualLength);
			BeamVisualMaterial->SetScalarParameterValue(TEXT("BeamTanHalfAngle"), FMath::Tan(FMath::DegreesToRadians(BeamHalfAngleDeg)));
			BeamVisualMaterial->SetVectorParameterValue(TEXT("BeamTint"), BeamColor);
		}
	}
	if (BeamLensMesh)
	{
		if (UMaterialInstanceDynamic* const LensMaterial = BeamLensMesh->CreateAndSetMaterialInstanceDynamic(0))
		{
			LensMaterial->SetVectorParameterValue(TEXT("LensTint"), BeamColor);
			LensMaterial->SetScalarParameterValue(TEXT("LensIntensity"), 0.85f);
		}
	}

	if (bStartInAutoRotation)
	{
		SetRotationMode(EBBBeamRotationMode::Auto);
	}
	else
	{
		SetRotationMode(EBBBeamRotationMode::Off);
	}

	// Tick stays enabled for the component's life: even when the beam is
	// idle, reveal objects need time-driven updates so things reveal while
	// the beam rests on them, and fade when the lantern dies.
	SetComponentTickEnabled(true);
	ApplyVisibleState();
}

void UBBLighthouseBeamComponent::SetRotationMode(EBBBeamRotationMode InMode)
{
	if (RotationMode == InMode)
	{
		return;
	}
	RotationMode = InMode;
	OnRotationModeChanged.Broadcast(RotationMode);
}

void UBBLighthouseBeamComponent::SetPowered(bool bInPowered)
{
	if (bPowered == bInPowered)
	{
		return;
	}
	bPowered = bInPowered;

	// Without power the lantern is dead - no light, no rotation. Ticking
	// stays enabled so reveal objects learn about the dying beam.
	if (!bPowered)
	{
		IntensityCurrent = 0.0f;
	}

	PublishBeamState();
	ApplyVisibleState();
}

void UBBLighthouseBeamComponent::SetManualYawTarget(float YawDegrees)
{
	TargetYawDeg = YawDegrees;
	if (RotationMode != EBBBeamRotationMode::Manual)
	{
		SetRotationMode(EBBBeamRotationMode::Manual);
	}
}

void UBBLighthouseBeamComponent::SetManualPitchDegrees(float PitchDegrees)
{
	CurrentPitchDeg = FMath::Clamp(PitchDegrees, -20.0f, 20.0f);
}

void UBBLighthouseBeamComponent::SetBeamIntensityTarget(float Intensity01)
{
	IntensityTarget = FMath::Clamp(Intensity01, 0.0f, 1.0f);
}

void UBBLighthouseBeamComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Reveal objects are driven every tick while the machine exists: the
	// beam resting on something is exactly when reveals must accumulate.
	const BlackBeacon::Logics::FBBBeamQuery Query = BuildQuery();
	UpdateReveals(Query, DeltaTime);

	if (!bPowered)
	{
		return; // dead lantern: nothing moves
	}

	const bool bYawChanged = UpdateRotation(DeltaTime);
	const bool bIntensityChanged = UpdateIntensity(DeltaTime);
	if (bYawChanged || bIntensityChanged)
	{
		PublishBeamState();
		ApplyVisibleState();
	}
}

bool UBBLighthouseBeamComponent::UpdateRotation(float DeltaTime)
{
	const float PreviousYaw = CurrentYawDeg;

	switch (RotationMode)
	{
	case EBBBeamRotationMode::Off:
		break;

	case EBBBeamRotationMode::Auto:
		// The classic patrol sweep: continuous rotation.
		CurrentYawDeg += AutoRotationDegPerSec * DeltaTime;
		break;

	case EBBBeamRotationMode::Manual:
		{
			// Player aim: ease toward the requested heading at a human rate.
			const float DeltaYaw = FMath::FindDeltaAngleDegrees(CurrentYawDeg, TargetYawDeg);
			const float MaxStep = ManualRotationDegPerSec * DeltaTime;
			const float Step = FMath::Clamp(DeltaYaw, -MaxStep, MaxStep);
			CurrentYawDeg = FRotator::NormalizeAxis(CurrentYawDeg + Step);
		}
		break;
	}

	// Normalize after auto sweeps grow large.
	CurrentYawDeg = FRotator::NormalizeAxis(CurrentYawDeg);

	return !FMath::IsNearlyEqual(PreviousYaw, CurrentYawDeg, 1e-3f);
}

bool UBBLighthouseBeamComponent::UpdateIntensity(float DeltaTime)
{
	// Mechanical flicker: a slow periodic dip when configured.
	float FlickerFactor = 1.0f;
	if (FlickerFrequency > 0.0f)
	{
		FlickerPhase += DeltaTime * FlickerFrequency;
		const float Wave = FMath::Sin(FlickerPhase * UE_TWO_PI);
		FlickerFactor = 1.0f - FlickerAmplitude * FMath::Max(0.0f, Wave);
	}

	const float Ground = bPowered ? FlickerFactor : 0.0f;
	const float Target = IntensityTarget * Ground;
	constexpr float IntensityResponse = 8.0f; // smooth but prompt hunt
	IntensityCurrent = FMath::FInterpConstantTo(IntensityCurrent, Target, DeltaTime, IntensityResponse);

	return !FMath::IsNearlyEqual(IntensityCurrent, Target, 1e-4f);
}

BlackBeacon::Logics::FBBBeamQuery UBBLighthouseBeamComponent::BuildQuery() const
{
	BlackBeacon::Logics::FBBBeamQuery Query;
	Query.Origin = BlackBeacon::Logics::BBVec3(
		GetComponentLocation().X,
		GetComponentLocation().Y,
		GetComponentLocation().Z);

	const FRotator BeamRotation(CurrentPitchDeg, CurrentYawDeg, 0.0f);
	const FVector Forward = BeamRotation.Vector();
	Query.Direction = BlackBeacon::Logics::BBVec3(Forward.X, Forward.Y, Forward.Z);

	Query.HalfAngleRad = FMath::DegreesToRadians(BeamHalfAngleDeg);
	Query.Intensity01 = bPowered ? FMath::Clamp(IntensityCurrent, 0.0f, 1.0f) : 0.0f;
	Query.RangeCm = BeamRangeCm;
	Query.bPowered = bPowered;
	return Query;
}

void UBBLighthouseBeamComponent::PublishBeamState()
{
	const BlackBeacon::Logics::FBBBeamQuery Query = BuildQuery();
	OnBeamQueryChanged.Broadcast(Query);
	ApplyVisibleState();
}

void UBBLighthouseBeamComponent::UpdateReveals(const BlackBeacon::Logics::FBBBeamQuery& Query, float DeltaTime)
{
	// Small, explicit subscription list - never a world search. Stale weak
	// refs are cleaned inline as they are encountered.
	for (int32 I = SubscribedReveals.Num() - 1; I >= 0; --I)
	{
		UBBBeamRevealComponent* const Reveal = SubscribedReveals[I].Get();
		if (Reveal)
		{
			Reveal->UpdateFromBeam(Query, DeltaTime);
		}
		else
		{
			SubscribedReveals.RemoveAtSwap(I, 1, EAllowShrinking::No);
		}
	}
}

void UBBLighthouseBeamComponent::ApplyVisibleState()
{
	if (!BeamLight)
	{
		return;
	}

	const bool bUseful = bPowered && IntensityCurrent > 0.01f;
	BeamLight->SetVisibility(bUseful);
	BeamOriginGlow->SetVisibility(bUseful);
	BeamLensMesh->SetVisibility(bUseful);

	// Scale the visible light with current intensity (lumens fall off with
	// flicker sag so the fog volume "breathes" with the machine).
	BeamLight->SetIntensity(BeamMaxIntensityLumens * IntensityCurrent);
	BeamVisualPivot->SetRelativeRotation(FRotator(CurrentPitchDeg, CurrentYawDeg, 0.0f));
	if (BeamVisualMesh)
	{
		BeamVisualMesh->SetVisibility(bUseful);
	}
	if (BeamVisualMaterial)
	{
		BeamVisualMaterial->SetScalarParameterValue(TEXT("BeamOpacity"), BeamVisualOpacity * IntensityCurrent);
		const FVector Origin = GetComponentLocation();
		const FVector Direction = FRotator(CurrentPitchDeg, CurrentYawDeg, 0.0f).Vector();
		BeamVisualMaterial->SetVectorParameterValue(TEXT("BeamOriginWS"), FLinearColor(Origin.X, Origin.Y, Origin.Z, 1.0f));
		BeamVisualMaterial->SetVectorParameterValue(TEXT("BeamDirectionWS"), FLinearColor(Direction.X, Direction.Y, Direction.Z, 1.0f));
	}
}

BlackBeacon::Logics::FBBBeamQuery UBBLighthouseBeamComponent::GetBeamQuery() const
{
	return BuildQuery();
}

void UBBLighthouseBeamComponent::SubscribeRevealComponent(UBBBeamRevealComponent* Reveal)
{
	if (!Reveal)
	{
		return;
	}
	if (!SubscribedReveals.Contains(Reveal))
	{
		SubscribedReveals.Add(Reveal);
	}
}

void UBBLighthouseBeamComponent::UnsubscribeRevealComponent(UBBBeamRevealComponent* Reveal)
{
	SubscribedReveals.RemoveSingle(Reveal);
}
