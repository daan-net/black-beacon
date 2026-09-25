#include "BlackBeacon/Lighthouse/BBLighthouseController.h"

#include "GameFramework/PlayerController.h"
#include "Internationalization/Text.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

#include "BlackBeacon/Lighthouse/BBLighthouseBeamComponent.h"
#include "BlackBeacon/Lighthouse/BBHeroArchitectureComponent.h"
#include "BlackBeacon/Lighthouse/BBLighthouseCollisionComponent.h"
#include "BlackBeacon/Lighthouse/BBBeamControlComponent.h"
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
	UStaticMesh* const HeroTowerMesh = LoadObject<UStaticMesh>(nullptr,
		TEXT("/Game/BlackBeacon/Art/Lighthouse/Meshes/SM_BB_LH_TowerShell.SM_BB_LH_TowerShell"));
	if (HeroTowerMesh)
	{
		TowerExteriorSkin->SetStaticMesh(HeroTowerMesh);
		TowerExteriorSkin->SetRelativeLocation(FVector(0.0f, 0.0f, -1980.0f));
		TowerExteriorSkin->SetRelativeScale3D(FVector::OneVector);
	}
	const auto AddHeroVisual = [this](const TCHAR* Name, const TCHAR* AssetPath, const FVector& RelativeLocation)
	{
		UStaticMeshComponent* const Visual = CreateDefaultSubobject<UStaticMeshComponent>(FName(Name));
		Visual->SetupAttachment(RootComponent);
		Visual->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, AssetPath));
		Visual->SetRelativeLocation(RelativeLocation);
		Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Visual->SetCanEverAffectNavigation(false);
		Visual->SetCastShadow(true);
		HeroVisualComponents.Add(Visual);
	};
	AddHeroVisual(TEXT("HeroGallery"),
		TEXT("/Game/BlackBeacon/Art/Lighthouse/Meshes/SM_BB_LH_Gallery.SM_BB_LH_Gallery"),
		FVector(0.0f, 0.0f, -1980.0f));
	AddHeroVisual(TEXT("HeroLanternRoom"),
		TEXT("/Game/BlackBeacon/Art/Lighthouse/Meshes/SM_BB_LH_LanternRoom.SM_BB_LH_LanternRoom"),
		FVector::ZeroVector);
	AddHeroVisual(TEXT("HeroRockPlinth"),
		TEXT("/Game/BlackBeacon/Art/Lighthouse/Meshes/SM_BB_LH_RockPlinth.SM_BB_LH_RockPlinth"),
		FVector(0.0f, 0.0f, -1980.0f));
	LanternFrame = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("LanternFrame"));
	LanternFrame->SetupAttachment(RootComponent);
	LanternFrame->SetStaticMesh(CubeMesh);
	LanternFrame->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LanternFrame->SetCastShadow(false);
	LanternFrame->SetCanEverAffectNavigation(false);
	LanternFresnelBands = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("LanternFresnelBands"));
	LanternFresnelBands->SetupAttachment(RootComponent);
	LanternFresnelBands->SetStaticMesh(CubeMesh);
	LanternFresnelBands->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LanternFresnelBands->SetCastShadow(false);
	LanternFresnelBands->SetCanEverAffectNavigation(false);
	LanternFresnelBands->SetVisibility(false);
	LanternFresnelBands->SetMaterial(0, LanternLitMaterial);
	LanternRoof = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LanternRoof"));
	LanternRoof->SetupAttachment(RootComponent);
	LanternRoof->SetStaticMesh(ConeMesh);
	LanternRoof->SetRelativeLocation(FVector(0.0f, 0.0f, 148.0f));
	LanternRoof->SetRelativeScale3D(FVector(3.4f, 3.4f, 1.2f));
	LanternRoof->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LanternRoof->SetCastShadow(false);
	LanternRoof->SetMaterial(0, LanternDarkMaterial);
	LanternRoof->SetVisibility(false);

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
			SideRadial * 243.0f + FVector(0.0f, 0.0f, -64.0f), FVector(1.98f, 0.025f, 3.60f)));
		GlassPanel->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GlassPanel->SetCastShadow(false);
		GlassPanel->SetCanEverAffectNavigation(false);
		GlassPanel->SetMaterial(0, LanternDarkMaterial);
		LanternGlazingPanels.Add(GlassPanel);

		for (const float BandHeight : {-64.0f, -32.0f, 0.0f, 32.0f, 64.0f})
		{
			LanternFresnelBands->AddInstance(FTransform(FRotator(0.0f, SideYaw, 0.0f),
				SideRadial * 70.0f + FVector(0.0f, 0.0f, BandHeight - 2.0f),
				FVector(0.48f, 0.025f, 0.025f)));
		}

		for (const float RailHeight : {-102.0f, 102.0f})
		{
			LanternFrame->AddInstance(FTransform(FRotator(0.0f, SideYaw, 0.0f),
				SideRadial * 146.1f + FVector(0.0f, 0.0f, RailHeight), FVector(1.10f, 0.10f, 0.10f)));
		}
	}
	LanternRoof->SetVisibility(false);

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
    UBBHeroArchitectureComponent* Architecture = NewObject<UBBHeroArchitectureComponent>(this, TEXT("HeroArchitecture"));
    AddInstanceComponent(Architecture);
    Architecture->RegisterComponent();
    auto* Collision = NewObject<UBBLighthouseCollisionComponent>(this, TEXT("LighthouseCollision"));
    AddInstanceComponent(Collision);
    Collision->RegisterComponent();
    LanternFrame->SetVisibility(false);
	if (TowerExteriorSkin && LanternDarkMaterial)
	{
		UMaterialInterface* const TowerMaterial = LoadObject<UMaterialInterface>(nullptr,
			TEXT("/Game/BlackBeacon/Materials/M_WetBasaltRock.M_WetBasaltRock"));
		if (TowerMaterial)
		{
			TowerExteriorSkin->SetMaterial(0, TowerMaterial);
		}
		if (UMaterialInstanceDynamic* const Material = TowerExteriorSkin->CreateAndSetMaterialInstanceDynamic(0))
		{
			UTexture2D* const LighthousePaint = LoadObject<UTexture2D>(nullptr,
				TEXT("/Game/BlackBeacon/Textures/T_LighthousePaintAlbedo.T_LighthousePaintAlbedo"));
			if (LighthousePaint)
			{
				Material->SetTextureParameterValue(TEXT("RockAlbedo"), LighthousePaint);
			}
			Material->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.72f, 0.75f, 0.78f));
			Material->SetScalarParameterValue(TEXT("Metallic"), 0.0f);
			Material->SetScalarParameterValue(TEXT("Specular"), 0.30f);
			Material->SetScalarParameterValue(TEXT("Roughness"), 0.86f);
		}
	}
	if (LanternFresnelBands)
	{
		FresnelBandsMaterial = LanternFresnelBands->CreateAndSetMaterialInstanceDynamic(0);
		if (FresnelBandsMaterial)
		{
			FresnelBandsMaterial->SetVectorParameterValue(TEXT("LensTint"), FLinearColor(1.0f, 0.46f, 0.16f));
			FresnelBandsMaterial->SetScalarParameterValue(TEXT("LensIntensity"), 0.55f);
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

	if (bChanged && bPowered)
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
        if (InteractingController)
        {
            if (UBBBeamControlComponent* Control = InteractingController->FindComponentByClass<UBBBeamControlComponent>())
            {
                if (IsBeamInManualMode()) Control->Acquire(this);
                else Control->Release();
            }
        }
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
	if (LanternFresnelBands)
	{
		LanternFresnelBands->SetVisibility(bLanternLit);
	}
	const auto SetSurface = [this](UStaticMeshComponent* Mesh, UMaterialInterface* Base,
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
				if (Base == LanternLitMaterial)
				{
					Material->SetVectorParameterValue(TEXT("LensTint"), Color);
					Material->SetScalarParameterValue(TEXT("LensIntensity"), 0.60f);
				}
				Material->SetScalarParameterValue(TEXT("Roughness"), Roughness);
			}
		}
	};
	SetSurface(LanternFrame, LanternDarkMaterial, FLinearColor(0.018f, 0.024f, 0.030f), 0.78f);
	for (UStaticMeshComponent* const GlassPanel : LanternGlazingPanels)
	{
		SetSurface(GlassPanel, LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/BlackBeacon/Art/Lighthouse/Materials/M_V04_Glazing.M_V04_Glazing")),
			bLanternLit ? FLinearColor(0.72f, 0.30f, 0.065f) : FLinearColor(0.012f, 0.018f, 0.026f),
			bLanternLit ? 0.48f : 0.82f);
	}
}
