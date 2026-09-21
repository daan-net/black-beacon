#include "BlackBeacon/Lighthouse/BBLighthouseBeamComponent.h"

#include "Components/LightComponentBase.h"
#include "Components/SpotLightComponent.h"
#include "Engine/World.h"

#include "BlackBeacon/Lighthouse/BBBeamRevealComponent.h"

UBBLighthouseBeamComponent::UBBLighthouseBeamComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;

	// Visible representation: a movable spotlight. In fog (volumetric fog
	// enabled in DefaultEngine.ini) this reads as a solid beam; the final
	// lens/glow presentation in 0.2 binds Niagara/light-shaft assets off
	// the same intensity/color/cone values this component drives.
	BeamLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("BeamLight"));
	BeamLight->SetupAttachment(this);
	BeamLight->SetMobility(EComponentMobility::Movable);
	BeamLight->SetCastShadows(true);
}

void UBBLighthouseBeamComponent::BeginPlay()
{
	Super::BeginPlay();

	if (BeamLight)
	{
		BeamLight->SetLightUnits(ELightUnits::Lumens);
		BeamLight->SetIntensity(BeamMaxIntensityLumens);
		BeamLight->SetLightColor(BeamColor);
		BeamLight->SetAttenuationRadius(BeamRangeCm);
		BeamLight->SetOuterConeAngle(BeamHalfAngleDeg);
		BeamLight->SetInnerConeAngle(BeamHalfAngleDeg * 0.5f);
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
			SubscribedReveals.RemoveAtSwap(I, 1, /*bAllowShrinking=*/false);
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

	// Scale the visible light with current intensity (lumens fall off with
	// flicker sag so the fog volume "breathes" with the machine).
	BeamLight->SetIntensity(BeamMaxIntensityLumens * IntensityCurrent);
	BeamLight->SetRelativeRotation(FRotator(CurrentPitchDeg, CurrentYawDeg, 0.0f));
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