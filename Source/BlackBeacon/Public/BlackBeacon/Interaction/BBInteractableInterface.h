// BLACK BEACON - interaction interface.
//
// Everything the player can interact with implements this interface.
// Implemented strictly in C++ for 0.1 (no Blueprint implementers needed);
// the interaction component focuses the nearest actor whose component
// implements it and calls OnInteract on confirm.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "BBInteractableInterface.generated.h"

class APlayerController;

UINTERFACE(MinimalAPI)
class UBBInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

class IBBInteractableInterface
{
	GENERATED_BODY()

public:
	// Player-facing prompt text, e.g. "Start Generator" / "Operate Beam Control".
	virtual FText GetInteractionPrompt() const = 0;

	// Called when the player confirms focus on this interactable.
	virtual void OnInteract(APlayerController* InteractingController) = 0;
};