// BLACK BEACON - player interaction component.
//
// Periodically traces forward from the player's camera (10 Hz, NOT every
// frame - rule: no wasteful ticking), remembers the focused interactable,
// and drives an interaction-prompt event. The player controller binds the
// "Interact" input to TryInteract().

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "BBInteractionComponent.generated.h"

class UCameraComponent;
class IBBInteractableInterface;

DECLARE_MULTICAST_DELEGATE_TwoParams(FBBInteractionFocusChanged, AActor* /*FocusedActor*/, const FText& /*Prompt*/);

UCLASS(ClassGroup = (BlackBeacon), meta = (BlueprintSpawnableComponent))
class UBBInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBBInteractionComponent();

	// Attempts to interact with the currently focused actor.
	UFUNCTION(BlueprintCallable, Category = "BlackBeacon|Interaction")
	bool TryInteract();

	AActor* GetFocusedActor() const { return FocusedActor.Get(); }
	bool HasFocus() const { return IsValid(FocusedActor.Get()); }

	// Fired whenever the focused interactable changes (or focus is lost).
	FBBInteractionFocusChanged OnFocusChanged;

	UPROPERTY(EditAnywhere, Category = "BlackBeacon|Interaction")
	float TraceDistance = 600.0f;

	UPROPERTY(EditAnywhere, Category = "BlackBeacon|Interaction")
	float TraceInterval = 0.1f; // 10 Hz - plenty for a prompt

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// Resolution context is the owning player character's camera.
	UCameraComponent* GetSourceCamera() const;
	void RefreshFocus(); // timer callback
	void ClearFocus();
	void ApplyFocus(AActor* NewActor);

	UPROPERTY()
	TObjectPtr<AActor> FocusedActor = nullptr;

	FTimerHandle FocusTimerHandle;
	bool bTryInteractQueued = false;
};
