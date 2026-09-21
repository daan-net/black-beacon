#include "BlackBeacon/Core/BBPlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

#include "BlackBeacon/Interaction/BBInteractionComponent.h"

ABBlackBeaconPlayerCharacter::ABBlackBeaconPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true; // stance/FOV smoothing only (single pawn)

	GetCapsuleComponent()->InitCapsuleSize(38.0f, 88.0f);

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->TargetArmLength = 0.0f; // first-person: camera at the boom tip

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(CameraBoom);
	CameraComponent->bUsePawnControlRotation = false; // boom carries rotation

	InteractionComponent = CreateDefaultSubobject<UBBInteractionComponent>(TEXT("InteractionComponent"));

	UCharacterMovementComponent* const Move = GetCharacterMovement();
	Move->MaxWalkSpeed = WalkSpeed;
	Move->MaxWalkSpeedCrouched = CrouchSpeed;
	Move->BrakingDecelerationWalking = 1200.0f;
}

void ABBlackBeaconPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	CameraStandZ = CameraBoom->GetRelativeLocation().Z;
	CameraCrouchZ = CameraStandZ - (StandHalfHeight - CrouchHalfHeight);
}

void ABBlackBeaconPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateStance(DeltaSeconds);
}

void ABBlackBeaconPlayerCharacter::UpdateStance(float DeltaSeconds)
{
	const bool bWantsSprint = bSprinting && !bWantsCrouch;
	UCharacterMovementComponent* const Move = GetCharacterMovement();

	if (Move)
	{
		Move->MaxWalkSpeed = bWantsCrouch
			? CrouchSpeed
			: (bWantsSprint ? SprintSpeed : WalkSpeed);
	}

	// Crouch: lower the capsule instantly (collision) and glide the eye down.
	if (bWantsCrouch)
	{
		if (GetCapsuleComponent()->GetScaledCapsuleHalfHeight() > CrouchHalfHeight + 1.0f)
		{
			GetCapsuleComponent()->SetCapsuleSize(GetCapsuleComponent()->GetUnscaledCapsuleRadius(), CrouchHalfHeight);
		}
	}
	else if (GetCapsuleComponent()->GetScaledCapsuleHalfHeight() < StandHalfHeight - 1.0f)
	{
		GetCapsuleComponent()->SetCapsuleSize(GetCapsuleComponent()->GetUnscaledCapsuleRadius(), StandHalfHeight);
	}

	if (CameraBoom)
	{
		const float TargetZ = bWantsCrouch ? CameraCrouchZ : CameraStandZ;
		const float CurrentZ = CameraBoom->GetRelativeLocation().Z;
		const float NewZ = FMath::FInterpTo(CurrentZ, TargetZ, DeltaSeconds, 12.0f);
		CameraBoom->SetRelativeLocation(FVector(0.0f, 0.0f, NewZ));
	}

	// Sprint FOV kick.
	if (CameraComponent)
	{
		const float TargetFov = bWantsSprint ? SprintFov : BaseFov;
		CameraComponent->SetFieldOfView(FMath::FInterpTo(CameraComponent->FieldOfView, TargetFov, DeltaSeconds, 10.0f));
	}
}

void ABBlackBeaconPlayerCharacter::StartSprint()
{
	bSprinting = true;
}

void ABBlackBeaconPlayerCharacter::StopSprint()
{
	bSprinting = false;
}

void ABBlackBeaconPlayerCharacter::SetCrouched(bool bNewCrouched)
{
	bWantsCrouch = bNewCrouched;
	if (bWantsCrouch)
	{
		bSprinting = false; // sprint and crouch are mutually exclusive
	}
}