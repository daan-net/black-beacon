#include "BlackBeacon/Core/BBCoastalEnvironment.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

namespace
{
	constexpr const TCHAR* CUBE_MESH = TEXT("/Engine/BasicShapes/Cube");
	constexpr const TCHAR* CYLINDER_MESH = TEXT("/Engine/BasicShapes/Cylinder");
	constexpr const TCHAR* SPHERE_MESH = TEXT("/Engine/BasicShapes/Sphere");
	constexpr const TCHAR* COAST_MATERIAL = TEXT("/Game/BlackBeacon/Materials/M_CoastSurface.M_CoastSurface");

	struct FCoastShape
	{
		FVector Location;
		FRotator Rotation;
		FVector Scale;
	};

	const FCoastShape ROCKS[] =
	{
		{{-4700, -1450,  20}, { 12,  18, -8}, {7.5f, 4.0f, 1.8f}},
		{{-4200,  1150, -10}, {-10, -12, 11}, {6.0f, 3.5f, 1.5f}},
		{{-3450, -1350,  35}, { 18,  30, -5}, {5.0f, 3.0f, 1.7f}},
		{{-2850,  1250,  15}, {-16,  15,  9}, {4.8f, 2.8f, 1.4f}},
		{{-2050, -1200,  40}, { 10, -20,  7}, {4.2f, 2.6f, 1.8f}},
		{{-1350,  1050,  45}, {-14,  26, -6}, {4.5f, 2.4f, 1.6f}},
		{{ -650, -1050,  20}, { 12, -15, 10}, {4.0f, 2.8f, 1.5f}},
		{{  150,   950,  10}, {-10,  20, -8}, {5.5f, 3.0f, 1.7f}},
		{{-5050,  -300, -75}, {  0,   0,  0}, {5.0f, 5.0f, 1.0f}},
		{{  150,     0,-130}, {  0,   0,  0}, {8.5f, 8.5f, 2.2f}},
	};

	const FCoastShape PATH_STONES[] =
	{
		{{-4300, -520,  7}, {0,  8, 0}, {3.8f, 1.8f, 0.10f}},
		{{-3600, -420,  8}, {0,  5, 0}, {3.6f, 1.7f, 0.11f}},
		{{-2900, -310,  8}, {0, 10, 0}, {3.6f, 1.6f, 0.10f}},
		{{-2200, -205,  8}, {0,  6, 0}, {3.5f, 1.5f, 0.11f}},
		{{-1500, -120,  8}, {0,  4, 0}, {3.5f, 1.5f, 0.10f}},
		{{ -800,  -55,  8}, {0,  3, 0}, {3.4f, 1.5f, 0.11f}},
	};

	const FCoastShape WRECKAGE[] =
	{
		{{-4800, -760,  65}, { 8,  18,  22}, {3.2f, 0.12f, 0.16f}},
		{{-4610, -910,  45}, {-4, -32, -18}, {2.2f, 0.10f, 0.13f}},
		{{-4470, -980,  32}, {12,  55,  10}, {1.4f, 0.09f, 0.11f}},
	};
}

ABBCoastalEnvironment::ABBCoastalEnvironment()
{
	PrimaryActorTick.bCanEverTick = false;
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	for (int32 Index = 0; Index < UE_ARRAY_COUNT(ROCKS); ++Index)
	{
		const FCoastShape& Shape = ROCKS[Index];
		RockSurfaces.Add(AddShape(
			*FString::Printf(TEXT("Rock_%02d"), Index),
			Index >= 8 ? CYLINDER_MESH : SPHERE_MESH,
			Shape.Location, Shape.Rotation, Shape.Scale, false));
	}

	for (int32 Index = 0; Index < UE_ARRAY_COUNT(PATH_STONES); ++Index)
	{
		const FCoastShape& Shape = PATH_STONES[Index];
		PathSurfaces.Add(AddShape(
			*FString::Printf(TEXT("Path_%02d"), Index), SPHERE_MESH,
			Shape.Location, Shape.Rotation, Shape.Scale, false));
	}

	for (int32 Index = 0; Index < UE_ARRAY_COUNT(WRECKAGE); ++Index)
	{
		const FCoastShape& Shape = WRECKAGE[Index];
		WreckSurfaces.Add(AddShape(
			*FString::Printf(TEXT("Wreck_%02d"), Index), CUBE_MESH,
			Shape.Location, Shape.Rotation, Shape.Scale, false));
	}
}

void ABBCoastalEnvironment::BeginPlay()
{
	Super::BeginPlay();

	UMaterialInterface* const BaseMaterial = LoadObject<UMaterialInterface>(nullptr, COAST_MATERIAL);
	auto ApplyWetMaterial = [this, BaseMaterial](
		const TArray<TObjectPtr<UStaticMeshComponent>>& Components,
		const FLinearColor& Color,
		float Roughness)
	{
		for (UStaticMeshComponent* const Component : Components)
		{
			if (!Component || !BaseMaterial)
			{
				continue;
			}
			UMaterialInstanceDynamic* const Material = UMaterialInstanceDynamic::Create(BaseMaterial, this);
			Material->SetVectorParameterValue(TEXT("BaseColor"), Color);
			Material->SetScalarParameterValue(TEXT("Roughness"), Roughness);
			Component->SetMaterial(0, Material);
		}
	};

	ApplyWetMaterial(RockSurfaces, FLinearColor(0.012f, 0.018f, 0.022f), 0.24f);
	ApplyWetMaterial(PathSurfaces, FLinearColor(0.022f, 0.028f, 0.030f), 0.16f);
	ApplyWetMaterial(WreckSurfaces, FLinearColor(0.028f, 0.016f, 0.009f), 0.34f);
}

UStaticMeshComponent* ABBCoastalEnvironment::AddShape(
	const TCHAR* Name,
	const TCHAR* MeshPath,
	const FVector& Location,
	const FRotator& Rotation,
	const FVector& Scale,
	bool bCollision)
{
	UStaticMeshComponent* const Component = CreateDefaultSubobject<UStaticMeshComponent>(Name);
	Component->SetupAttachment(SceneRoot);
	Component->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, MeshPath));
	Component->SetRelativeLocation(Location);
	Component->SetRelativeRotation(Rotation);
	Component->SetRelativeScale3D(Scale);
	Component->SetCollisionEnabled(bCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
	Component->SetCastShadow(true);
	return Component;
}
