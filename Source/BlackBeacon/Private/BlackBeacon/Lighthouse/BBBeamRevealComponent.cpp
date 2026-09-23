#include "BlackBeacon/Lighthouse/BBBeamRevealComponent.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"

#include "BlackBeacon/Lighthouse/BBLighthouseBeamComponent.h"
#include "BlackBeacon/Lighthouse/BBLighthouseController.h"
#include "BlackBeacon/Objectives/BBObjectiveSystem.h"

UBBBeamRevealComponent::UBBBeamRevealComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UBBBeamRevealComponent::BeginPlay()
{
	Super::BeginPlay();

	BlackBeacon::Logics::FBBRevealParams Params;
	Params.RevealDelay = RevealDelay;
	Params.FadeTime = FadeTime;
	Params.MinBeamIntensity = MinBeamIntensity;
	Params.VisibilityDuration = VisibilityDuration;
	Params.bPersistent = bPersistent;
	Machine.SetParams(Params);

	// A hidden object must not block the player either.
	AActor* const Owner = GetOwner();
	if (Owner)
	{
		Owner->SetActorHiddenInGame(true);
		Owner->SetActorEnableCollision(false);
		FadeMesh = Owner->FindComponentByClass<UStaticMeshComponent>();
	}

	if (bAutoSubscribeToBeam)
	{
		LocateBeam();
	}
}

void UBBBeamRevealComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UBBLighthouseBeamComponent* const BeamPtr = Beam.Get())
	{
		BeamPtr->UnsubscribeRevealComponent(this);
	}
	Beam = nullptr;
	Super::EndPlay(EndPlayReason);
}

void UBBBeamRevealComponent::LocateBeam()
{
	// One lookup at load time (not per frame): find the lighthouse in this
	// world. The greybox bootstrap spawns exactly one.
	UWorld* const World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* const Candidate = *It;
		if (Candidate && Candidate->IsA<ABBLighthouseController>())
		{
			if (UBBLighthouseBeamComponent* const Found = Candidate->FindComponentByClass<UBBLighthouseBeamComponent>())
			{
				Beam = Found;
				Found->SubscribeRevealComponent(this);
				return;
			}
		}
	}

	// Dev worlds may spawn the lighthouse after this component (bootstrap
	// races). Retry a few times, cheaply, then give up.
	if (AutoSubscribeAttempts < MaxAutoSubscribeAttempts)
	{
		++AutoSubscribeAttempts;
		FTimerHandle RetryHandle;
		GetWorld()->GetTimerManager().SetTimer(
			RetryHandle, FTimerDelegate::CreateWeakLambda(this, [this]() { LocateBeam(); }),
			0.5f, /*bLoop=*/false);
	}
}

void UBBBeamRevealComponent::UpdateFromBeam(const BlackBeacon::Logics::FBBBeamQuery& BeamQuery, float DeltaTime)
{
	AActor* const Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	const bool bWasFullyRevealed = Machine.WasFullyRevealed();

	Machine.Tick(DeltaTime, BeamQuery, BlackBeacon::Logics::BBVec3(
		Owner->GetActorLocation().X,
		Owner->GetActorLocation().Y,
		Owner->GetActorLocation().Z));

	ApplyVisibility(Machine.GetVisibilityAmount());

	if (!bWasFullyRevealed && Machine.WasFullyRevealed())
	{
		HandleFirstFullReveal();
	}
}

void UBBBeamRevealComponent::ApplyVisibility(float Amount)
{
	AActor* const Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	const bool bVisible = Amount > 0.01f;
	Owner->SetActorHiddenInGame(!bVisible);
	Owner->SetActorEnableCollision(bVisible);

	// Optional material cross-fade (only when the asset material exposes the
	// named scalar; otherwise the visibility toggle carries the reveal).
	if (FadeMesh && !FadeMaterialParameterName.IsNone())
	{
		UMaterialInstanceDynamic* const MIC = FadeMesh->CreateAndSetMaterialInstanceDynamic(0);
		if (MIC)
		{
			MIC->SetScalarParameterValue(FadeMaterialParameterName, Amount);
		}
	}
}

void UBBBeamRevealComponent::HandleFirstFullReveal()
{
	if (bCompletedCallbackFired)
	{
		return;
	}
	bCompletedCallbackFired = true;
	OnFullyRevealed.Broadcast();

	if (bTriggerObjective && !ObjectiveId.IsNone())
	{
		if (UWorld* const World = GetWorld())
		{
			if (UGameInstance* const GI = World->GetGameInstance())
			{
				if (UBBObjectiveSystem* const Objectives = GI->GetSubsystem<UBBObjectiveSystem>())
				{
					Objectives->CompleteObjective(ObjectiveId.ToString());
				}
			}
		}
	}
}
void UBBBeamRevealComponent::ForceReveal()
{
	Machine.ForceReveal();
	ApplyVisibility(1.0f);
	HandleFirstFullReveal();
}
