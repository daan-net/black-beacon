#include "BlackBeacon/Lighthouse/BBLighthouseController.h"

#include "GameFramework/PlayerController.h"
#include "Internationalization/Text.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

#include "BlackBeacon/Lighthouse/BBLighthouseBeamComponent.h"
#include "BlackBeacon/Objectives/BBObjectiveSystem.h"
#include "BlackBeacon/Power/BBPowerSystem.h"

ABBLighthouseController::ABBLighthouseController()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	UStaticMesh* const CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
	UStaticMesh* const ConeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cone"));
	UStaticMesh* const CylinderMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder"));
	LanternLitMaterial = LoadObject<UMaterialInterface>(
		nullptr, TEXT("/Game/BlackBeacon/Materials/M_LanternLens.M_LanternLens"));
	LanternDarkMaterial = LoadObject<UMaterialInterface>(
		nullptr, TEXT("/Game/BlackBeacon/Materials/M_CoastSurface.M_CoastSurface"));
	TowerExteriorSkin = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TowerExteriorSkin"));
	TowerExteriorSkin->SetupAttachment(RootComponent);
	TowerExteriorSkin->SetStaticMesh(CylinderMesh);
	TowerExteriorSkin->SetRelativeLocation(FVector(0.0f, 0.0f, -1200.0f));
	TowerExteriorSkin->SetRelativeScale3D(FVector(6.76f, 6.76f, 15.6f));
	TowerExteriorSkin->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TowerExteriorSkin->SetCastShadow(true);
	TowerExteriorSkin->SetCanEverAffectNavigation(false);
	TowerExteriorSkin->SetMaterial(0, LoadObject<UMaterialInterface>(
		nullptr, TEXT("/Game/BlackBeacon/Materials/M_WetBasaltRock.M_WetBasaltRock")));
	LanternFrame = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("LanternFrame"));
	LanternFrame->SetupAttachment(RootComponent);
	LanternFrame->SetStaticMesh(CubeMesh);
	LanternFrame->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LanternFrame->SetCastShadow(false);
	LanternFrame->SetCanEverAffectNavigation(false);
	UStaticMeshComponent* const LanternRoof = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LanternRoof"));
	LanternRoof->SetupAttachment(RootComponent);
	LanternRoof->SetStaticMesh(ConeMesh);
	LanternRoof->SetRelativeLocation(FVector(0.0f, 0.0f, 148.0f));
	LanternRoof->SetRelativeScale3D(FVector(3.4f, 3.4f, 1.2f));
	LanternRoof->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LanternRoof->SetCastShadow(false);
	LanternRoof->SetMaterial(0, LanternDarkMaterial);

	for (int32 Panel = 0; Panel < 8; ++Panel)
	{
		const float AngleDeg = static_cast<float>(Panel) * 45.0f;
		const float AngleRad = FMath::DegreesToRadians(AngleDeg);
		const float SideMidpointAngle = AngleRad + UE_PI / 8.0f;
		const float SideMidpointDegrees = AngleDeg + 22.5f;
		const FVector Radial(FMath::Cos(AngleRad), FMath::Sin(AngleRad), 0.0f);
		LanternFrame->AddInstance(FTransform(FRotator(0.0f, AngleDeg, 0.0f),
			Radial * 158.0f, FVector(0.09f, 0.09f, 1.9f)));

		const FVector SideRadial(FMath::Cos(SideMidpointAngle), FMath::Sin(SideMidpointAngle), 0.0f);
		const float SideYaw = SideMidpointDegrees + 90.0f;
		UStaticMeshComponent* const GlassPanel = CreateDefaultSubobject<UStaticMeshComponent>(
			*FString::Printf(TEXT("LanternGlass_%d"), Panel));
		GlassPanel->SetupAttachment(RootComponent);
		GlassPanel->SetStaticMesh(CubeMesh);
		GlassPanel->SetRelativeTransform(FTransform(FRotator(0.0f, SideYaw, 0.0f),
			SideRadial * 146.1f + FVector(0.0f, 0.0f, -2.0f), FVector(1.10f, 0.035f, 1.75f)));
		GlassPanel->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GlassPanel->SetCastShadow(false);
		GlassPanel->SetCanEverAffectNavigation(false);
		GlassPanel->SetMaterial(0, LanternDarkMaterial);
		LanternGlazingPanels.Add(GlassPanel);

		for (const float RailHeight : {-102.0f, 102.0f})
		{
			LanternFrame->AddInstance(FTransform(FRotator(0.0f, SideYaw, 0.0f),
				SideRadial * 146.1f + FVector(0.0f, 0.0f, RailHeight), FVector(1.10f, 0.10f, 0.10f)));
		}
	}

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
	if (TowerExteriorSkin && LanternDarkMaterial)
	{
		if (UMaterialInstanceDynamic* const Material = TowerExteriorSkin->CreateAndSetMaterialInstanceDynamic(0))
		{
			Material->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.11f, 0.14f, 0.17f));
			Material->SetScalarParameterValue(TEXT("Roughness"), 0.84f);
		}
	}
	RegisterWithPowerSystem();
	UpdateLanternHousingState();
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
	UpdateLanternHousingState();

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
	UpdateLanternHousingState();
}

void ABBLighthouseController::RequestBeamStart()
{
	if (!BeamComponent || !bPowered || bBeamStarted)
	{
		return;
	}

	bBeamStarted = true;
	UpdateLanternHousingState();

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

void ABBLighthouseController::UpdateLanternHousingState()
{
	if (!LanternFrame || LanternGlazingPanels.IsEmpty())
	{
		return;
	}
	const bool bLanternLit = bPowered && bBeamStarted;
	const auto SetSurface = [](UStaticMeshComponent* Mesh, UMaterialInterface* Base,
		const FLinearColor& Color, float Roughness)
	{
		if (Mesh && Base)
		{
			if (Mesh->GetMaterial(0) != Base)
			{
				Mesh->SetMaterial(0, Base);
			}
		if (UMaterialInstanceDynamic* const Material = Mesh->CreateAndSetMaterialInstanceDynamic(0))
			{
				Material->SetVectorParameterValue(TEXT("BaseColor"), Color);
				Material->SetScalarParameterValue(TEXT("Roughness"), Roughness);
			}
		}
	};
	SetSurface(LanternFrame, LanternDarkMaterial, FLinearColor(0.018f, 0.024f, 0.030f), 0.68f);
	for (UStaticMeshComponent* const GlassPanel : LanternGlazingPanels)
	{
		SetSurface(GlassPanel, bLanternLit ? LanternLitMaterial : LanternDarkMaterial,
			bLanternLit ? FLinearColor(1.0f, 0.56f, 0.22f) : FLinearColor(0.012f, 0.018f, 0.026f),
			bLanternLit ? 0.25f : 0.62f);
	}
}
