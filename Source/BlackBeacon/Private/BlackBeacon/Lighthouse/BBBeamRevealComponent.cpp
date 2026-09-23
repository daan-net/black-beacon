#include "BlackBeacon/Lighthouse/BBBeamRevealComponent.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Materials/MaterialInstanceDynamic.h"

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
	if (FadeMesh && !FadeMaterialParameterName.IsNone())
	{
		FadeMaterial = FadeMesh->CreateAndSetMaterialInstanceDynamic(0);
	}
	CacheTaggedParts();

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

	const bool bWasFullyRevealed = WasFullyRevealed();
	if (bUseTaggedPartReveal && !TaggedPartStates.IsEmpty())
	{
		for (FTaggedPartState& Part : TaggedPartStates)
		{
			if (UStaticMeshComponent* const Mesh = Part.Mesh.Get())
			{
				const FVector Position = Mesh->Bounds.Origin;
				Part.Machine.Tick(DeltaTime, BeamQuery,
					BlackBeacon::Logics::BBVec3(Position.X, Position.Y, Position.Z));
			}
		}
		ApplyTaggedPartVisibility();
		if (!bWasFullyRevealed && WasFullyRevealed())
		{
			HandleFirstFullReveal();
		}
		return;
	}

	Machine.Tick(DeltaTime, BeamQuery, BlackBeacon::Logics::BBVec3(
		Owner->GetActorLocation().X,
		Owner->GetActorLocation().Y,
		Owner->GetActorLocation().Z));

	ApplyVisibility(Machine.GetVisibilityAmount());

	if (!bWasFullyRevealed && WasFullyRevealed())
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
		if (FadeMaterial)
		{
			FadeMaterial->SetScalarParameterValue(FadeMaterialParameterName, Amount);
		}
	}
}

void UBBBeamRevealComponent::EnableTaggedPartReveal()
{
	bUseTaggedPartReveal = true;
	Machine.Reset();
	CacheTaggedParts();
}

void UBBBeamRevealComponent::CacheTaggedParts()
{
	if (!bUseTaggedPartReveal)
	{
		return;
	}

	AActor* const Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	TArray<UStaticMeshComponent*> TaggedMeshes;
	Owner->GetComponents<UStaticMeshComponent>(TaggedMeshes);
	TaggedMeshes.RemoveAll([this](const UStaticMeshComponent* Mesh)
	{
		return !Mesh || !Mesh->ComponentHasTag(RevealPartTag);
	});

	if (TaggedMeshes.Num() == TaggedPartStates.Num())
	{
		bool bSameParts = true;
		for (int32 Index = 0; Index < TaggedMeshes.Num(); ++Index)
		{
			bSameParts &= TaggedPartStates[Index].Mesh.Get() == TaggedMeshes[Index];
		}
		if (bSameParts)
		{
			return;
		}
	}

	BlackBeacon::Logics::FBBRevealParams Params;
	Params.RevealDelay = RevealDelay;
	Params.FadeTime = FadeTime;
	Params.MinBeamIntensity = MinBeamIntensity;
	Params.VisibilityDuration = VisibilityDuration;
	Params.bPersistent = bPersistent;

	TaggedPartStates.Reset(TaggedMeshes.Num());
	for (UStaticMeshComponent* const Mesh : TaggedMeshes)
	{
		FTaggedPartState& Part = TaggedPartStates.AddDefaulted_GetRef();
		Part.Mesh = Mesh;
		Part.Machine.SetParams(Params);
		Part.Material = Cast<UMaterialInstanceDynamic>(Mesh->GetMaterial(0));
		if (!Part.Material.IsValid())
		{
			Part.Material = Mesh->CreateAndSetMaterialInstanceDynamic(0);
		}
		Mesh->SetVisibility(false, false);
	}
	ApplyTaggedPartVisibility();
}

void UBBBeamRevealComponent::ApplyTaggedPartVisibility()
{
	AActor* const Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	bool bAnyVisible = false;
	for (FTaggedPartState& Part : TaggedPartStates)
	{
		const float Amount = static_cast<float>(Part.Machine.GetVisibilityAmount());
		const bool bPartVisible = Amount > 0.01f;
		if (UStaticMeshComponent* const Mesh = Part.Mesh.Get())
		{
			Mesh->SetVisibility(bPartVisible, false);
		}
		if (UMaterialInstanceDynamic* const Material = Part.Material.Get())
		{
			Material->SetVectorParameterValue(TEXT("BaseColor"), RevealedPartTint * Amount);
		}
		bAnyVisible |= bPartVisible;
	}
	Owner->SetActorHiddenInGame(!bAnyVisible);
	Owner->SetActorEnableCollision(false);
}

bool UBBBeamRevealComponent::WasFullyRevealed() const
{
	if (!bUseTaggedPartReveal || TaggedPartStates.IsEmpty())
	{
		return Machine.WasFullyRevealed();
	}
	for (const FTaggedPartState& Part : TaggedPartStates)
	{
		if (Part.Machine.WasFullyRevealed())
		{
			return true;
		}
	}
	return false;
}

float UBBBeamRevealComponent::GetVisibilityAmount() const
{
	if (!bUseTaggedPartReveal || TaggedPartStates.IsEmpty())
	{
		return static_cast<float>(Machine.GetVisibilityAmount());
	}
	float MaxAmount = 0.0f;
	for (const FTaggedPartState& Part : TaggedPartStates)
	{
		MaxAmount = FMath::Max(MaxAmount, static_cast<float>(Part.Machine.GetVisibilityAmount()));
	}
	return MaxAmount;
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
	if (bUseTaggedPartReveal && !TaggedPartStates.IsEmpty())
	{
		for (FTaggedPartState& Part : TaggedPartStates)
		{
			Part.Machine.ForceReveal();
		}
		ApplyTaggedPartVisibility();
		HandleFirstFullReveal();
		return;
	}
	Machine.ForceReveal();
	ApplyVisibility(1.0f);
	HandleFirstFullReveal();
}

void UBBBeamRevealComponent::ResetForRestore()
{
	Machine.Reset();
	for (FTaggedPartState& Part : TaggedPartStates)
	{
		Part.Machine.Reset();
	}
	bCompletedCallbackFired = false;
	if (bUseTaggedPartReveal && !TaggedPartStates.IsEmpty())
	{
		ApplyTaggedPartVisibility();
	}
	else
	{
		ApplyVisibility(0.0f);
	}
}

void UBBBeamRevealComponent::SetRevealedForRestore(bool bRevealed)
{
	if (bRevealed)
	{
		if (bUseTaggedPartReveal && !TaggedPartStates.IsEmpty())
		{
			for (FTaggedPartState& Part : TaggedPartStates)
			{
				Part.Machine.ForceReveal();
			}
			ApplyTaggedPartVisibility();
		}
		else
		{
			Machine.ForceReveal();
			ApplyVisibility(1.0f);
		}
		bCompletedCallbackFired = true;
	}
	else
	{
		ResetForRestore();
	}
}
