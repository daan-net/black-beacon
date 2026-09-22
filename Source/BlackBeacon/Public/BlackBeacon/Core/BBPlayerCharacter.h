// BLACK BEACON - player character.
//
// A walking/looking/sprinting/crouching investigator. Movement tuning and
// stance state live here; input translation lives in the controller.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "BBPlayerCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UBBInteractionComponent;

UCLASS(config = Game)
class ABBlackBeaconPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABBlackBeaconPlayerCharacter();

	// --- stances / verbs ---
	UFUNCTION(BlueprintCallable, Category = "BlackBeacon|Player")
	void StartSprint();

	UFUNCTION(BlueprintCallable, Category = "BlackBeacon|Player")
	void StopSprint();

	UFUNCTION(BlueprintCallable, Category = "BlackBeacon|Player")
	void SetCrouched(bool bNewCrouched);

	UFUNCTION(BlueprintCallable, Category = "BlackBeacon|Player")
	bool IsSprinting() const { return bSprinting; }

	UFUNCTION(BlueprintCallable, Category = "BlackBeacon|Player")
	bool IsCrouchedByPlayer() const { return bWantsCrouch; }

	// --- access (interaction component resolves its trace from the camera) ---
	UCameraComponent* GetFirstPersonCamera() const { return CameraComponent; }
	UBBInteractionComponent* GetInteractionComponent() const { return InteractionComponent; }

	// --- component refs ---
	UPROPERTY(VisibleAnywhere, Category = "BlackBeacon|Player")
	TObjectPtr<USpringArmComponent> CameraBoom = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "BlackBeacon|Player")
	TObjectPtr<UCameraComponent> CameraComponent = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "BlackBeacon|Player")
	TObjectPtr<UBBInteractionComponent> InteractionComponent = nullptr;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	void UpdateStance(float DeltaSeconds);

	UPROPERTY(config, EditAnywhere, Category = "BlackBeacon|Player|Movement")
	float WalkSpeed = 400.0f;

	UPROPERTY(config, EditAnywhere, Category = "BlackBeacon|Player|Movement")
	float SprintSpeed = 700.0f;

	UPROPERTY(config, EditAnywhere, Category = "BlackBeacon|Player|Movement")
	float CrouchSpeed = 190.0f;

	UPROPERTY(config, EditAnywhere, Category = "BlackBeacon|Player|Movement")
	float StandEyeHeightCm = 152.0f;

	UPROPERTY(config, EditAnywhere, Category = "BlackBeacon|Player|Movement")
	float CrouchEyeHeightCm = 100.0f;

	UPROPERTY(EditAnywhere, Category = "BlackBeacon|Player|Movement")
	float BaseFov = 80.0f;

	UPROPERTY(EditAnywhere, Category = "BlackBeacon|Player|Movement")
	float SprintFov = 88.0f;

	float StandHalfHeight = 88.0f;
	float CrouchHalfHeight = 56.0f;

	bool bSprinting = false;
	bool bWantsCrouch = false;
	float CameraStandZ = 0.0f;
	float CameraCrouchZ = 0.0f;
};
