#include "BlackBeacon/Core/BBPlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

#include "BlackBeacon/Interaction/BBInteractionComponent.h"
#include "BlackBeacon/Logics/BBStairAssist.h"

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
	Move->BrakingDecelerationWalking = 2200.0f;
	Move->GroundFriction = 10.0f;
	Move->GetNavAgentPropertiesRef().bCanCrouch = true;
	Move->SetCrouchedHalfHeight(CrouchHalfHeight);
	Move->bCrouchMaintainsBaseLocation = true;
}

void ABBlackBeaconPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	CameraStandZ = StandEyeHeightCm - StandHalfHeight;
	CameraCrouchZ = CrouchEyeHeightCm - CrouchHalfHeight;
	CameraBoom->SetRelativeLocation(FVector(0.0f, 0.0f, CameraStandZ));
}

void ABBlackBeaconPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
    const FVector Position = GetActorLocation();
    const bool bAssist = IsStandingOnStairTread() && Position.Z-88.0f<1550.0f;
    if (Controller && bAssist)
    {
        using BlackBeacon::Logics::BBVec3;
        const double Delta = BlackBeacon::Logics::FBBStairAssist::TravelYaw(
            BBVec3(previousStairPosition.X,previousStairPosition.Y,previousStairPosition.Z),
            BBVec3(Position.X,Position.Y,Position.Z), bStairForwardInput && !GetVelocity().IsNearlyZero());
        FRotator View = Controller->GetControlRotation();
        View.Yaw += Delta;
        Controller->SetControlRotation(View);
    }
    previousStairPosition = Position;
    bStairForwardInput = false;
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
			: (IsStandingOnStairTread() ? StairWalkSpeed : (bWantsSprint ? SprintSpeed : WalkSpeed));
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
		Crouch();
	}
	else
	{
		UnCrouch();
	}
}

bool ABBlackBeaconPlayerCharacter::IsStandingOnStairTread() const
{
	const UCharacterMovementComponent* const Movement = GetCharacterMovement();
	const UPrimitiveComponent* const Base = Movement
		? Cast<UPrimitiveComponent>(Movement->GetMovementBaseObject())
		: nullptr;
	const AActor* const BaseActor = Base ? Base->GetOwner() : nullptr;
	return BaseActor && BaseActor->ActorHasTag(TEXT("BB_StairStep"));
}

FVector ABBlackBeaconPlayerCharacter::AssistedForward(const FVector& Forward, const FVector2D& Input)
{
    bStairForwardInput = FMath::Abs(Input.Y)>0.1 && FMath::Abs(Input.X)<0.1;
    if (!IsStandingOnStairTread() || GetActorLocation().Z-88.0f>=1550.0f) return Forward;
    using BlackBeacon::Logics::BBVec3;
    const FVector Position = GetActorLocation();
    const auto Result = BlackBeacon::Logics::FBBStairAssist::CenteredForward(
        BBVec3(Forward.X,Forward.Y,0), BBVec3(Position.X,Position.Y,Position.Z),
        Position.Z-88.0f,Input.Y,Input.X);
    return FVector(Result.X,Result.Y,Result.Z);
}
