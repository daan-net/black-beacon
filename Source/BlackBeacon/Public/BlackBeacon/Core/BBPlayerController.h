// BLACK BEACON - player controller.
//
// Enhanced Input lives entirely in code (no .uasset input assets): the
// mapping context and actions are created at runtime, which keeps the
// project text-driven and auditable. Also owns the interaction prompt UI.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "BBPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;
class UBBPromptWidget;

UCLASS(config = Game)
class ABBlackBeaconPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ABBlackBeaconPlayerController();

	// The prompt widget (created in BeginPlay from the widget class).
	UPROPERTY(VisibleAnywhere, Category = "BlackBeacon|UI")
	TObjectPtr<UBBPromptWidget> PromptWidget = nullptr;

	FTimerHandle NotificationTimer;

	// Manual beam aim hook (0.1: horizontal look sweeps the lantern while
	// the player is manning the beam - see HandleLook).
	UFUNCTION(BlueprintCallable, Category = "BlackBeacon|Player")
	void SetManualBeamYaw(float YawDegrees);

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	// --- input wiring ---
	void CreateInputAssets();

	// --- action handlers ---
	void HandleMove(const struct FInputActionValue& Value);
	void HandleLook(const FInputActionValue& Value);
	void HandleSprintStarted();
	void HandleSprintCompleted();
	void HandleCrouch();
	void HandleInteract();
	void HandleSave();
	void HandleLoad();

	// --- prompt UI ---
	void CreatePromptWidget();
	void OnInteractionFocusChanged(AActor* FocusedActor, const FText& Prompt);
	void OnUiObjectiveChanged(const FString& Id, const FString& Text);
	void ClearNotification();

	// Held at runtime (created in CreateInputAssets) so GC keeps them alive.
	UPROPERTY()
	TObjectPtr<UInputAction> MoveAction = nullptr;

	UPROPERTY()
	TObjectPtr<UInputAction> LookAction = nullptr;

	UPROPERTY()
	TObjectPtr<UInputAction> SprintAction = nullptr;

	UPROPERTY()
	TObjectPtr<UInputAction> CrouchAction = nullptr;

	UPROPERTY()
	TObjectPtr<UInputAction> InteractAction = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "BlackBeacon|Input")
	TObjectPtr<UInputAction> SaveAction = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "BlackBeacon|Input")
	TObjectPtr<UInputAction> LoadAction = nullptr;

	UPROPERTY()
	TObjectPtr<UInputMappingContext> MappingContext = nullptr;

	UPROPERTY()
	TObjectPtr<class UBBInteractionComponent> InteractionComponent = nullptr;

	UPROPERTY()
	TObjectPtr<class ABBlackBeaconPlayerCharacter> PossessedCharacter = nullptr;

	// Cached when the player takes control of the beam (O(1) manual aim).
	UPROPERTY()
	TObjectPtr<class ABBLighthouseController> CachedBeamController = nullptr;

	UPROPERTY(config, EditAnywhere, Category = "BlackBeacon|Player|Aim")
	float MouseYawDegreesPerCount = 0.35f;

	UPROPERTY(config, EditAnywhere, Category = "BlackBeacon|Player|Aim")
	float MousePitchDegreesPerCount = 0.18f;

	UPROPERTY(config, EditAnywhere, Category = "BlackBeacon|Player|Movement", meta = (ClampMin = "-5.0", ClampMax = "5.0"))
	float StairPitchLockDegrees = 0.0f;

	UPROPERTY(config, EditAnywhere, Category = "BlackBeacon|Player|Aim")
	float ManualAimSensitivity = 0.22f;

	UPROPERTY(config, EditAnywhere, Category = "BlackBeacon|Player|Opening")
	FRotator OpeningViewRotation = FRotator(-2.0f, 8.0f, 0.0f);

	UPROPERTY(config, EditAnywhere, Category = "BlackBeacon|Player|Opening", meta = (ClampMin = "0.0"))
	float OpeningFadeSeconds = 2.5f;
};
