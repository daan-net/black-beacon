#include "BlackBeacon/Interaction/BBInteractionComponent.h"

#include "Camera/CameraComponent.h"
#include "CollisionQueryParams.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"

#include "BlackBeacon/Interaction/BBInteractableInterface.h"

UBBInteractionComponent::UBBInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

namespace
{
	// The focused interactable may live on the actor itself or on one of
	// its components (e.g. UBBGeneratorComponent attached to a mesh actor).
	IBBInteractableInterface* FindInteractableOnActor(AActor* Actor)
	{
		if (!Actor)
		{
			return nullptr;
		}

		if (Actor->GetClass()->ImplementsInterface(UBBInteractableInterface::StaticClass()))
		{
			return Cast<IBBInteractableInterface>(Actor);
		}

		for (UActorComponent* Component : Actor->GetComponents())
		{
			if (Component && Component->GetClass()->ImplementsInterface(UBBInteractableInterface::StaticClass()))
			{
				return Cast<IBBInteractableInterface>(Component);
			}
		}
		return nullptr;
	}
}

void UBBInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* const Owner = GetOwner();
	if (Owner)
	{
		Owner->GetWorldTimerManager().SetTimer(
			FocusTimerHandle, this, &UBBInteractionComponent::RefreshFocus,
			TraceInterval, /*bLoop=*/true, /*InitialDelay=*/0.05f);
	}
}

void UBBInteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AActor* const Owner = GetOwner())
	{
		Owner->GetWorldTimerManager().ClearTimer(FocusTimerHandle);
	}
	ClearFocus();
	Super::EndPlay(EndPlayReason);
}

UCameraComponent* UBBInteractionComponent::GetSourceCamera() const
{
	AActor* const Owner = GetOwner();
	return Owner ? Owner->FindComponentByClass<UCameraComponent>() : nullptr;
}

void UBBInteractionComponent::RefreshFocus()
{
	UCameraComponent* const Camera = GetSourceCamera();
	if (!Camera)
	{
		return;
	}

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(BBInteractionTrace), /*bTraceComplex=*/false);
	Params.AddIgnoredActor(GetOwner());

	const FVector Start = Camera->GetComponentLocation();
	const FVector End = Start + Camera->GetForwardVector() * TraceDistance;

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		if (FindInteractableOnActor(Hit.GetActor()))
		{
			ApplyFocus(Hit.GetActor());
			return;
		}
	}

	ClearFocus();
}

void UBBInteractionComponent::ApplyFocus(AActor* NewActor)
{
	FText Prompt = FText::GetEmpty();
	if (NewActor)
	{
		if (IBBInteractableInterface* const Interactable = FindInteractableOnActor(NewActor))
		{
			Prompt = Interactable->GetInteractionPrompt();
		}
	}
	if (FocusedActor == NewActor && FocusedPrompt.EqualTo(Prompt))
	{
		return;
	}
	FocusedActor = NewActor;
	FocusedPrompt = Prompt;
	OnFocusChanged.Broadcast(NewActor, Prompt);
}

void UBBInteractionComponent::ClearFocus()
{
	if (!IsValid(FocusedActor.Get()))
	{
		return;
	}
	FocusedActor = nullptr;
	FocusedPrompt = FText::GetEmpty();
	OnFocusChanged.Broadcast(nullptr, FText::GetEmpty());
}

bool UBBInteractionComponent::TryInteract()
{
	AActor* const Actor = FocusedActor.Get();
	if (!Actor)
	{
		return false;
	}

	IBBInteractableInterface* const Interactable = FindInteractableOnActor(Actor);
	if (!Interactable)
	{
		return false;
	}

	APlayerController* PC = nullptr;
	if (const APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		PC = Cast<APlayerController>(OwnerPawn->GetController());
	}

	// Retarget the focus check so stale focus can't fire through walls:
	// the interactable itself is trusted to gate on its own state.
	Interactable->OnInteract(PC);
	if (IsValid(Actor))
	{
		ApplyFocus(Actor);
	}
	else
	{
		ClearFocus();
	}
	return true;
}
