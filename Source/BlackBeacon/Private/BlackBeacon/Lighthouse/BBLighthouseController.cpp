#include "BlackBeacon/Lighthouse/BBLighthouseController.h"

#include "GameFramework/PlayerController.h"
#include "Internationalization/Text.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

#include "BlackBeacon/Lighthouse/BBLighthouseBeamComponent.h"
#include "BlackBeacon/Objectives/BBObjectiveSystem.h"
#include "BlackBeacon/Power/BBPowerSystem.h"

ABBLighthouseController::ABBLighthouseController()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	UStaticMeshComponent* const ControlMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ControlMesh"));
	ControlMesh->SetupAttachment(RootComponent);
	ControlMesh->SetRelativeLocation(FVector(200.0f, 0.0f, -350.0f));
	ControlMesh->SetRelativeScale3D(FVector(0.8f, 1.2f, 1.0f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ControlShape(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (ControlShape.Succeeded())
	{
		ControlMesh->SetStaticMesh(ControlShape.Object);
	}

	BeamComponent = CreateDefaultSubobject<UBBLighthouseBeamComponent>(TEXT("BeamComponent"));
	BeamComponent->SetupAttachment(RootComponent);
	// The greybox builder repositions this actor to the lantern room.
}

void ABBLighthouseController::BeginPlay()
{
	Super::BeginPlay();
	RegisterWithPowerSystem();
}

void ABBLighthouseController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* const World = GetWorld())
	{
		if (UBBPowerSystem* const Power = World->GetSubsystem<UBBPowerSystem>())
		{
			Power->UnregisterConsumer(this);
		}
	}
	Super::EndPlay(EndPlayReason);
}

void ABBLighthouseController::RegisterWithPowerSystem()
{
	if (UWorld* const World = GetWorld())
	{
		if (UBBPowerSystem* const Power = World->GetSubsystem<UBBPowerSystem>())
		{
			Power->RegisterConsumer(this);
		}
	}
}

// --- power consumer ---

FName ABBLighthouseController::GetConsumerId() const
{
	return ConsumerId;
}

float ABBLighthouseController::GetDemandWatts() const
{
	return DemandWatts;
}

void ABBLighthouseController::NotifyPowerState(bool bPoweredNow, float SuppliedWatts)
{
	const bool bChanged = bPowered != bPoweredNow;
	bPowered = bPoweredNow;

	if (BeamComponent)
	{
		BeamComponent->SetPowered(bPowered && bBeamStarted);
	}

	if (bPowered && !bPowerGrantedOnce)
	{
		bPowerGrantedOnce = true;
		HandleFirstPower(SuppliedWatts);
	}

	if (bChanged)
	{
		// The "RESTORE POWER" beat is complete the moment the lantern works.
		if (UWorld* const World = GetWorld())
		{
			if (UGameInstance* const GI = World->GetGameInstance())
			{
				if (UBBObjectiveSystem* const Objectives = GI->GetSubsystem<UBBObjectiveSystem>())
				{
					Objectives->CompleteObjective(ObjectiveOnPowered.ToString());
				}
			}
		}
	}

	if (bAutoStartBeamWhenPowered && bPowered && !bBeamStarted)
	{
		RequestBeamStart();
	}
}

void ABBLighthouseController::HandleFirstPower(float SuppliedWatts)
{
	// Hook for audio/visual power-up beats in later milestones.
	// (0.1: presence of power is the event.)
}

// --- interactable ---

FText ABBLighthouseController::GetInteractionPrompt() const
{
	if (!bPowered)
	{
		return NSLOCTEXT("BlackBeacon", "BeamNoPowerPrompt", "Beam Control - no power");
	}

	if (!bBeamStarted)
	{
		return NSLOCTEXT("BlackBeacon", "BeamStartPrompt", "Start the Lighthouse Beam");
	}

	if (BeamComponent && BeamComponent->GetRotationMode() == EBBBeamRotationMode::Manual)
	{
		return NSLOCTEXT("BlackBeacon", "BeamReleasePrompt", "Release Beam Control");
	}

	return NSLOCTEXT("BlackBeacon", "BeamTakeControlPrompt", "Take Control of the Beam");
}

void ABBLighthouseController::OnInteract(APlayerController* InteractingController)
{
	if (bBeamStarted)
	{
		ToggleBeamControl();
		return;
	}

	if (!bPowered)
	{
		return; // nothing to do: the machine is dead without power
	}
	RequestBeamStart();
}

void ABBLighthouseController::ToggleBeamControl()
{
	if (!BeamComponent || !bPowered)
	{
		return;
	}

	if (BeamComponent->GetRotationMode() == EBBBeamRotationMode::Manual)
	{
		BeamComponent->SetRotationMode(EBBBeamRotationMode::Auto);
	}
	else
	{
		BeamComponent->SetRotationMode(EBBBeamRotationMode::Manual);

		// First time the player takes the wheel: "AIM / ROTATE BEAM" beat.
		if (!bAimObjectiveCompleted)
		{
			bAimObjectiveCompleted = true;
			if (UWorld* const World = GetWorld())
			{
				if (UGameInstance* const GI = World->GetGameInstance())
				{
					if (UBBObjectiveSystem* const Objectives = GI->GetSubsystem<UBBObjectiveSystem>())
					{
						Objectives->CompleteObjective(ObjectiveOnAim.ToString());
					}
				}
			}
		}
	}
}

bool ABBLighthouseController::IsBeamInManualMode() const
{
	return BeamComponent
		&& BeamComponent->GetRotationMode() == EBBBeamRotationMode::Manual;
}

void ABBLighthouseController::RestoreBeamState(
	bool bInBeamStarted, bool bInAimObjectiveCompleted,
	EBBBeamRotationMode InMode, float InYawDegrees, float InPitchDegrees)
{
	bBeamStarted = bInBeamStarted;
	bAimObjectiveCompleted = bInAimObjectiveCompleted;
	if (!BeamComponent)
	{
		return;
	}

	BeamComponent->SetPowered(bPowered && bBeamStarted);
	if (bBeamStarted && bPowered)
	{
		BeamComponent->SetManualYawTarget(InYawDegrees);
		BeamComponent->SetManualPitchDegrees(InPitchDegrees);
		BeamComponent->SetRotationMode(InMode == EBBBeamRotationMode::Off ? EBBBeamRotationMode::Auto : InMode);
	}
	else
	{
		BeamComponent->SetRotationMode(EBBBeamRotationMode::Off);
	}
}

void ABBLighthouseController::RequestBeamStart()
{
	if (!BeamComponent || !bPowered || bBeamStarted)
	{
		return;
	}

	bBeamStarted = true;

	if (!BeamComponent->IsPowered())
	{
		BeamComponent->SetPowered(true);
	}
	BeamComponent->SetRotationMode(EBBBeamRotationMode::Auto);

	// Pacing beats: starting the beam completes its objective; the player
	// turning it to manual completes the aim beat (beam component event).
	if (UWorld* const World = GetWorld())
	{
		if (UGameInstance* const GI = World->GetGameInstance())
		{
			if (UBBObjectiveSystem* const Objectives = GI->GetSubsystem<UBBObjectiveSystem>())
			{
				Objectives->CompleteObjective(ObjectiveOnStarted.ToString());
			}
		}
	}
}
