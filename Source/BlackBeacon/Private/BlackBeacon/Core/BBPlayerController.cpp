#include "BlackBeacon/Core/BBPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"

#include "BlackBeacon/Core/BBPlayerCharacter.h"
#include "BlackBeacon/Interaction/BBInteractionComponent.h"
#include "BlackBeacon/Interaction/BBPromptWidget.h"
#include "BlackBeacon/Lighthouse/BBLighthouseBeamComponent.h"
#include "BlackBeacon/Lighthouse/BBLighthouseController.h"

ABBlackBeaconPlayerController::ABBlackBeaconPlayerController()
{
	bShowMouseCursor = false;
}

void ABBlackBeaconPlayerController::BeginPlay()
{
	Super::BeginPlay();
	CreatePromptWidget();

	PossessedCharacter = Cast<ABBlackBeaconPlayerCharacter>(GetPawn());
	if (PossessedCharacter)
	{
		InteractionComponent = PossessedCharacter->GetInteractionComponent();
		if (InteractionComponent)
		{
			InteractionComponent->OnFocusChanged.AddUObject(this, &ABBlackBeaconPlayerController::OnInteractionFocusChanged);
		}
	}
}

void ABBlackBeaconPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	CreateInputAssets();

	if (UEnhancedInputLocalPlayerSubsystem* const Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(MappingContext, /*Priority=*/0);
	}

	if (UEnhancedInputComponent* const Enhanced = Cast<UEnhancedInputComponent>(InputComponent))
	{
		Enhanced->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABBlackBeaconPlayerController::HandleMove);
		Enhanced->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABBlackBeaconPlayerController::HandleLook);
		Enhanced->BindAction(SprintAction, ETriggerEvent::Started, this, &ABBlackBeaconPlayerController::HandleSprintStarted);
		Enhanced->BindAction(SprintAction, ETriggerEvent::Completed, this, &ABBlackBeaconPlayerController::HandleSprintCompleted);
		Enhanced->BindAction(CrouchAction, ETriggerEvent::Started, this, &ABBlackBeaconPlayerController::HandleCrouch);
		Enhanced->BindAction(InteractAction, ETriggerEvent::Started, this, &ABBlackBeaconPlayerController::HandleInteract);
	}
}

void ABBlackBeaconPlayerController::CreateInputAssets()
{
	// All input assets are constructed at runtime so the project stays free
	// of editor-only .uasset dependencies. Defaults match the standard
	// first-person layout; rebinding UI can edit these contexts later.

	MoveAction = NewObject<UInputAction>(this, TEXT("IA_Move"));
	MoveAction->ValueType = EInputActionValueType::Axis2D;

	LookAction = NewObject<UInputAction>(this, TEXT("IA_Look"));
	LookAction->ValueType = EInputActionValueType::Axis2D;

	SprintAction = NewObject<UInputAction>(this, TEXT("IA_Sprint"));
	SprintAction->ValueType = EInputActionValueType::Boolean;

	CrouchAction = NewObject<UInputAction>(this, TEXT("IA_Crouch"));
	CrouchAction->ValueType = EInputActionValueType::Boolean;

	InteractAction = NewObject<UInputAction>(this, TEXT("IA_Interact"));
	InteractAction->ValueType = EInputActionValueType::Boolean;

	MappingContext = NewObject<UInputMappingContext>(this, TEXT("IMC_Default"));
	auto MapMove = [this](FKey Key, bool bForwardAxis, bool bNegate)
	{
		FEnhancedActionKeyMapping& Mapping = MappingContext->MapKey(MoveAction, Key);
		if (bForwardAxis)
		{
			Mapping.Modifiers.Add(NewObject<UInputModifierSwizzleAxis>(MappingContext));
		}
		if (bNegate)
		{
			Mapping.Modifiers.Add(NewObject<UInputModifierNegate>(MappingContext));
		}
	};
	MapMove(EKeys::W, true, false);
	MapMove(EKeys::S, true, true);
	MapMove(EKeys::A, false, true);
	MapMove(EKeys::D, false, false);
	MapMove(EKeys::Up, true, false);
	MapMove(EKeys::Down, true, true);
	MapMove(EKeys::Left, false, true);
	MapMove(EKeys::Right, false, false);

	MappingContext->MapKey(LookAction, EKeys::MouseX);
	FEnhancedActionKeyMapping& MouseYMapping = MappingContext->MapKey(LookAction, EKeys::MouseY);
	MouseYMapping.Modifiers.Add(NewObject<UInputModifierSwizzleAxis>(MappingContext));

	MappingContext->MapKey(SprintAction, EKeys::LeftShift);

	MappingContext->MapKey(CrouchAction, EKeys::C);

	MappingContext->MapKey(InteractAction, EKeys::E);
}

void ABBlackBeaconPlayerController::HandleMove(const FInputActionValue& Value)
{
	if (!PossessedCharacter)
	{
		PossessedCharacter = Cast<ABBlackBeaconPlayerCharacter>(GetPawn());
		if (!PossessedCharacter)
		{
			return;
		}
	}

	const FVector2D Axis = Value.Get<FVector2D>();
	const FRotator YawRotation(0.0f, GetControlRotation().Yaw, 0.0f);
	const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	PossessedCharacter->AddMovementInput(Forward, Axis.Y);
	PossessedCharacter->AddMovementInput(Right, Axis.X);
}

void ABBlackBeaconPlayerController::HandleLook(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();

	// While manning the beam, horizontal look aims the lantern instead of
	// the camera (the lens becomes the "body"). Cached on take-control so
	// this is O(1) per frame - never a world search.
	if (CachedBeamController && CachedBeamController->IsPowered()
		&& CachedBeamController->IsBeamInManualMode())
	{
		if (UBBLighthouseBeamComponent* const Beam = CachedBeamController->BeamComponent)
		{
			Beam->SetManualYawTarget(Beam->GetCurrentYawDegrees() + Axis.X * ManualAimSensitivity);
		}
		return;
	}

	AddYawInput(Axis.X * MouseYawDegreesPerCount);
	AddPitchInput(Axis.Y * MousePitchDegreesPerCount);
}

void ABBlackBeaconPlayerController::HandleSprintStarted()
{
	if (PossessedCharacter)
	{
		PossessedCharacter->StartSprint();
	}
}

void ABBlackBeaconPlayerController::HandleSprintCompleted()
{
	if (PossessedCharacter)
	{
		PossessedCharacter->StopSprint();
	}
}

void ABBlackBeaconPlayerController::HandleCrouch()
{
	// Simple toggle; release-to-stand arrives with the next objective/input
	// pass in 0.2 if needed (0.1: hold behaviour is not required).
	if (PossessedCharacter)
	{
		PossessedCharacter->SetCrouched(!PossessedCharacter->IsCrouchedByPlayer());
	}
}

void ABBlackBeaconPlayerController::HandleInteract()
{
	if (!InteractionComponent)
	{
		return;
	}

	InteractionComponent->TryInteract();

	// Cache the beam control when the interaction was with the lighthouse so
	// manual aim is constant-time afterwards.
	AActor* const Focused = InteractionComponent->GetFocusedActor();
	CachedBeamController = Cast<ABBLighthouseController>(Focused);
}

void ABBlackBeaconPlayerController::SetManualBeamYaw(float YawDegrees)
{
	// Explicit aim API (used by the mouse-aim path and future controls).
	if (CachedBeamController && CachedBeamController->BeamComponent)
	{
		CachedBeamController->BeamComponent->SetManualYawTarget(YawDegrees);
	}
}

void ABBlackBeaconPlayerController::CreatePromptWidget()
{
	if (PromptWidget)
	{
		return;
	}

	PromptWidget = CreateWidget<UBBPromptWidget>(this, UBBPromptWidget::StaticClass());
	if (PromptWidget)
	{
		PromptWidget->AddToViewport(/*ZOrder=*/100);
		PromptWidget->SetPromptText(FText::GetEmpty());
	}
}

void ABBlackBeaconPlayerController::OnInteractionFocusChanged(AActor* FocusedActor, const FText& Prompt)
{
	if (PromptWidget)
	{
		PromptWidget->SetPromptText(Prompt);
	}
}
