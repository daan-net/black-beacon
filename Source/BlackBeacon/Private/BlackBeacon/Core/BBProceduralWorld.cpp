#include "BlackBeacon/Core/BBProceduralWorld.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"

#include "BlackBeacon/Lighthouse/BBBeamRevealComponent.h"
#include "BlackBeacon/Lighthouse/BBLighthouseController.h"
#include "BlackBeacon/Objectives/BBObjectiveTriggerComponent.h"
#include "BlackBeacon/Power/BBGeneratorComponent.h"
#include "BlackBeacon/Weather/BBWeatherController.h"

namespace
{
	constexpr const TCHAR* kMeshCube = TEXT("/Engine/BasicShapes/Cube");
	constexpr const TCHAR* kMeshCylinder = TEXT("/Engine/BasicShapes/Cylinder");
	constexpr const TCHAR* kMeshPlane = TEXT("/Engine/BasicShapes/Plane");

	UStaticMesh* LoadBasicShape(const TCHAR* Path)
	{
		return Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, Path));
	}
}

AActor* UBBProceduralWorld::SpawnMeshActor(
	UWorld* World,
	const TCHAR* BasicShapePath,
	const FTransform& Transform,
	FVector MeshScale,
	const FName& Tag)
{
	if (!World)
	{
		return nullptr;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* const Actor = World->SpawnActor<AActor>(AActor::StaticClass(), Transform, Params);
	if (!Actor)
	{
		return nullptr;
	}

	USceneComponent* const Root = NewObject<USceneComponent>(Actor, TEXT("Root"));
	Actor->SetRootComponent(Root);
	Root->SetWorldTransform(Transform);
	Root->RegisterComponent();

	UStaticMeshComponent* const Mesh = NewObject<UStaticMeshComponent>(Actor, TEXT("Mesh"));
	Mesh->SetupAttachment(Root);
	Mesh->SetStaticMesh(LoadBasicShape(BasicShapePath));
	Mesh->SetWorldScale3D(MeshScale);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetCollisionResponseToAllChannels(ECR_Block);
	Mesh->RegisterComponent();

	if (!Tag.IsNone())
	{
		Actor->Tags.Add(Tag);
	}
	return Actor;
}

AActor* UBBProceduralWorld::SpawnTriggerVolume(
	UWorld* World,
	const FName& ObjectiveId,
	const FVector& Center,
	const FVector& HalfExtent,
	bool bPlayerOnly)
{
	if (!World)
	{
		return nullptr;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* const Volume = World->SpawnActor<AActor>(AActor::StaticClass(), FTransform(Center), Params);
	if (!Volume)
	{
		return nullptr;
	}

	UBoxComponent* const Box = NewObject<UBoxComponent>(Volume, TEXT("TriggerBox"));
	Volume->SetRootComponent(Box);
	Box->SetWorldLocation(Center);
	Box->SetBoxExtent(HalfExtent);
	Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Box->SetCollisionResponseToAllChannels(ECR_Ignore);
	Box->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Box->SetGenerateOverlapEvents(true);
	Box->RegisterComponent();

	UBBObjectiveTriggerComponent* const Trigger = NewObject<UBBObjectiveTriggerComponent>(Volume, TEXT("ObjectiveTrigger"));
	Trigger->ObjectiveId = ObjectiveId;
	Trigger->TriggerType = EBBObjectiveTriggerType::EnterVolume;
	Trigger->bPlayerOnly = bPlayerOnly;
	Trigger->RegisterComponent();

	return Volume;
}

int32 UBBProceduralWorld::BuildSlice(UWorld* World)
{
	if (!World)
	{
		return 0;
	}

	int32 Spawned = 0;

	// --- terrain ---
	if (AActor* const Ground = SpawnMeshActor(
			World, kMeshPlane, FTransform(FRotator(0, 0, 0), FVector(0, 0, 0)),
			FVector(120.0f, 120.0f, 1.0f), TEXT("BB_Ground")))
	{
		++Spawned;
	}

	// Dark ocean surrounding the island (greybox - real water in 0.3).
	if (AActor* const Ocean = SpawnMeshActor(
			World, kMeshPlane, FTransform(FRotator(0, 0, 0), FVector(0, 0, -40.0f)),
			FVector(450.0f, 450.0f, 1.0f), TEXT("BB_Ocean")))
	{
		++Spawned;
	}

	// --- weather (storm baseline tuned in DefaultGame.ini) ---
	++Spawned; // weather actor is deterministic single instance

	// --- gameplay beats ---
	SpawnLighthouse(World);
	++Spawned;
	SpawnTowerAndStairs(World);
	++Spawned;
	SpawnGeneratorAnnex(World);
	++Spawned;
	SpawnRevealAnomaly(World);
	++Spawned;

	// --- objective chain: enter / find / climb volumes ---
	SpawnTriggerVolume(World, TEXT("BB_OBJ_ENTER_LIGHTHOUSE"), FVector(360.0f, 0.0f, 60.0f), FVector(220, 130, 160), true);
	SpawnTriggerVolume(World, TEXT("BB_OBJ_FIND_GENERATOR"), FVector(1560.0f, 1120.0f, 60.0f), FVector(220, 260, 180), true);
	SpawnTriggerVolume(World, TEXT("BB_OBJ_CLIMB"), FVector(0.0f, 0.0f, 1700.0f), FVector(500, 500, 160), true);
	Spawned += 3;

	return Spawned;
}

ABBWeatherController* UBBProceduralWorld::EnsureWeatherController(UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}

	// One controller per level.
	TActorIterator<ABBWeatherController> It(World);
	if (It)
	{
		return *It;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	return World->SpawnActor<ABBWeatherController>(ABBWeatherController::StaticClass(),
		FTransform(FRotator::ZeroRotator, FVector::ZeroVector), Params);
}

ABBLighthouseController* UBBProceduralWorld::SpawnLighthouse(UWorld* World)
{
	// Ensure the storm weather first: the lighthouse is the stage for it.
	EnsureWeatherController(World);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Controller sits at the lantern: tower floors at multiples of
	// kTowerFloorHeightCm, lantern floor at 3x + lamp headroom.
	const FVector LanternPosition(0.0f, 0.0f, kTowerFloorHeightCm * 3.0f + 420.0f);
	ABBLighthouseController* const Lighthouse = World->SpawnActor<ABBLighthouseController>(
		ABBLighthouseController::StaticClass(), FTransform(FRotator::ZeroRotator, LanternPosition), Params);
	return Lighthouse;
}

void UBBProceduralWorld::SpawnTowerAndStairs(UWorld* World)
{
	const UBBProceduralWorld* const Lighting = GetDefault<UBBProceduralWorld>();
	constexpr int32 kStepsPerFloor = 28;
	constexpr float kStepHeight = kTowerFloorHeightCm / static_cast<float>(kStepsPerFloor);
	constexpr float kTurnPerStepDeg = 360.0f / static_cast<float>(kStepsPerFloor);
	constexpr float kStairRadius = 200.0f;
	constexpr float kOuterRailRadius = 315.0f;

	// The central column closes the inner drop. The previous stairs had no
	// physical reference or protection at the open centre.
	SpawnMeshActor(
		World, kMeshCylinder,
		FTransform(FRotator::ZeroRotator, FVector(0.0f, 0.0f, kTowerFloorHeightCm * 1.5f)),
		FVector(1.8f, 1.8f, kTowerFloorHeightCm * 1.5f / 100.0f),
		TEXT("BB_StairCore"));

	// Three stacked cylindrical wall sections + floor discs (greybox).
	for (int32 Floor = 0; Floor < 3; ++Floor)
	{
		const float BaseZ = static_cast<float>(Floor) * kTowerFloorHeightCm;

		// Wall section (cylinder basic mesh: r=50, h=100 -> scale to radius &
		// height). Wall height slightly under floor height for a gap look.
		AActor* const Wall = SpawnMeshActor(
			World, kMeshCylinder,
			FTransform(FRotator::ZeroRotator, FVector(0, 0, BaseZ + kTowerFloorHeightCm * 0.5f)),
			FVector(kTowerRadiusCm / 50.0f, kTowerRadiusCm / 50.0f, kTowerFloorHeightCm * 0.5f / 100.0f),
			TEXT("BB_TowerWall"));
		(void)Wall;
		if (Wall)
		{
			Wall->SetActorEnableCollision(false);
			// One unshadowed fill per floor keeps the greybox route readable
			// without adding fixtures or changing stair collision.
			UPointLightComponent* const Fill = NewObject<UPointLightComponent>(Wall, TEXT("StairFillLight"));
			Fill->SetupAttachment(Wall->GetRootComponent());
			Fill->SetMobility(EComponentMobility::Movable);
			Fill->IntensityUnits = ELightUnits::Lumens;
			Fill->SetIntensity(Lighting->StairFillLumens);
			Fill->SetLightColor(Lighting->StairFillColor);
			Fill->SetAttenuationRadius(Lighting->StairFillRadiusCm);
			Fill->SetCastShadows(false);
			Fill->RegisterComponent();
		}

		// Floor disc (cube: 100x100x100 -> scale to a thin disc).
		if (Floor == 0)
		{
			SpawnMeshActor(
				World, kMeshCube,
				FTransform(FRotator::ZeroRotator, FVector(0, 0, BaseZ)),
				FVector(kTowerRadiusCm * 2.0f / 100.0f, kTowerRadiusCm * 2.0f / 100.0f, 0.2f),
				FName(TEXT("BB_TowerFloor")));
		}

		// Helical stair: boxes spiralling up inside the tower.
		for (int32 Step = 0; Step < kStepsPerFloor; ++Step)
		{
			const float StepZ = BaseZ + kStepHeight * (static_cast<float>(Step) + 0.5f);
			const float StepYawDeg = -static_cast<float>(Step) * kTurnPerStepDeg;
			const float AngleRad = FMath::DegreesToRadians(StepYawDeg);

			FVector StepPos(
				FMath::Cos(AngleRad) * kStairRadius,
				FMath::Sin(AngleRad) * kStairRadius,
				StepZ);

			FRotator StepRot(0.0f, StepYawDeg, 0.0f);

			// Radial depth is 240 cm and the walking run is 80 cm. These axes
			// were reversed in the original greybox, leaving only a 90 cm strip.
			SpawnMeshActor(
				World, kMeshCube,
				FTransform(StepRot, StepPos),
				FVector(2.4f, 0.8f, kStepHeight / 100.0f),
				TEXT("BB_StairStep"));

			const FVector RailPos(
				FMath::Cos(AngleRad) * kOuterRailRadius,
				FMath::Sin(AngleRad) * kOuterRailRadius,
				StepZ + 48.0f);
			SpawnMeshActor(
				World, kMeshCube,
				FTransform(StepRot, RailPos),
				FVector(0.12f, 0.8f, 0.95f),
				TEXT("BB_StairGuard"));
		}
	}

	// A compact landing leaves the stair opening clear and reaches the control.
	SpawnMeshActor(
		World, kMeshCube,
		FTransform(FRotator::ZeroRotator, FVector(200.0f, 0.0f, kTowerFloorHeightCm * 3.0f)),
		FVector(2.4f, 2.4f, 0.2f),
		TEXT("BB_LanternFloor"));
}

void UBBProceduralWorld::SpawnGeneratorAnnex(UWorld* World)
{
	// Generator shed: a box near the tower base with the generator inside.
	const FVector ShedCenter(1560.0f, 1120.0f, 150.0f);

	AActor* const Shed = SpawnMeshActor(
		World, kMeshCube,
		FTransform(FRotator(0, 25, 0), ShedCenter),
		FVector(4.2f, 5.2f, 2.6f),
		TEXT("BB_GeneratorShed"));
	(void)Shed;
	if (Shed)
	{
		Shed->SetActorEnableCollision(false);
	}

	// The generator itself (interactable, power source).
	AActor* const GenActor = SpawnMeshActor(
		World, kMeshCube,
		FTransform(FRotator(0, 0, 0), FVector(1560.0f, 1120.0f, 40.0f)),
		FVector(1.6f, 1.3f, 1.4f),
		TEXT("BB_Generator"));
	if (GenActor)
	{
		UBBGeneratorComponent* const Generator = NewObject<UBBGeneratorComponent>(GenActor, TEXT("Generator"));
		Generator->RegisterComponent();
	}
}

void UBBProceduralWorld::SpawnRevealAnomaly(UWorld* World)
{
	// The signature example: a derelict boathouse ruin on the far cliff.
	// Invisible normally; the beam rests on it and it materialises.
	const FVector Position(-5200.0f, 4200.0f, 80.0f);

	AActor* const Ruin = SpawnMeshActor(
		World, kMeshCube,
		FTransform(FRotator(0, -15, 0), Position),
		FVector(5.0f, 3.2f, 2.2f),
		TEXT("BB_Anomaly"));
	if (Ruin)
	{
		UBBBeamRevealComponent* const Reveal = NewObject<UBBBeamRevealComponent>(Ruin, TEXT("BeamReveal"));
		Reveal->RegisterComponent();
	}
}
