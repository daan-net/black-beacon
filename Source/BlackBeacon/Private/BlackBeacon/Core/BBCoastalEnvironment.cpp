#include "BlackBeacon/Core/BBCoastalEnvironment.h"

#include "Components/SceneComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "TimerManager.h"
#include "EngineUtils.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

#include "BlackBeacon/Power/BBGeneratorComponent.h"
#include "BlackBeacon/Objectives/BBObjectiveTriggerComponent.h"
#include "BlackBeacon/Lighthouse/BBBeamRevealComponent.h"
#include "BlackBeacon/Lighthouse/BBLighthouseController.h"

namespace
{
	constexpr const TCHAR* CUBE_MESH = TEXT("/Engine/BasicShapes/Cube");
	constexpr const TCHAR* CYLINDER_MESH = TEXT("/Engine/BasicShapes/Cylinder");
	constexpr const TCHAR* ROCK_MESH = TEXT("/PCG/SampleContent/SimpleForest/Meshes/PCG_Boulder_02.PCG_Boulder_02");
	constexpr const TCHAR* PLANE_MESH = TEXT("/Engine/BasicShapes/Plane");
	constexpr const TCHAR* COAST_MATERIAL = TEXT("/Game/BlackBeacon/Materials/M_CoastSurface.M_CoastSurface");
	constexpr const TCHAR* BASALT_MATERIAL = TEXT("/Game/BlackBeacon/Materials/M_WetBasaltRock.M_WetBasaltRock");
	constexpr const TCHAR* WRECK_HULL_TEXTURE = TEXT("/Game/BlackBeacon/Textures/T_WreckHullAlbedo.T_WreckHullAlbedo");
	constexpr const TCHAR* OCEAN_MATERIAL = TEXT("/Engine/EngineMaterials/WaterMaterial.DefaultWaterMaterial");
	constexpr float GENERATOR_SHED_LIGHT_LUMENS = 850.0f;
	constexpr float ROCK_SAMPLE_SCALE = 0.4f;
	const FVector GENERATOR_ANNEX_OLD_CENTER(1560.0f, 1120.0f, 0.0f);
	const FVector GENERATOR_ANNEX_HERO_CENTER(0.0f, -620.0f, 0.0f);

	struct FCoastShape
	{
		FVector Location;
		FRotator Rotation;
		FVector Scale;
	};

	const FCoastShape ROCKS[] = {
	{{-2500, -300, -80}, {0, 0, 0}, {60.0f, 60.0f, 1.0f}},
	{{-1803, -2462, -9}, {-25, 21, 244}, {7.46f, 3.43f, 7.66f}},
	{{-4851, -2172, 26}, {-43, -27, 234}, {5.72f, 4.10f, 8.92f}},
	{{-953, -2490, 71}, {18, -14, 56}, {7.79f, 4.68f, 5.20f}},
	{{-4516, -1229, 41}, {28, 21, 193}, {7.87f, 4.89f, 8.64f}},
	{{-853, -1572, 79}, {7, 18, 16}, {4.14f, 4.45f, 5.10f}},
	{{-3836, -2348, -8}, {12, -12, 133}, {4.05f, 4.33f, 11.52f}},
	{{-1760, -1586, -24}, {21, -30, 137}, {7.95f, 6.20f, 8.68f}},
	{{-1577, -1236, 66}, {-24, -42, 114}, {4.34f, 4.05f, 11.57f}},
	{{-618, -2028, 48}, {-9, 37, 165}, {4.32f, 4.23f, 8.71f}},
	{{-3686, -1623, 85}, {-9, -25, 359}, {5.55f, 3.45f, 4.85f}},
	{{-4452, -1559, 69}, {-7, -39, 137}, {7.98f, 5.65f, 11.78f}},
	{{-696, -2483, 58}, {16, 3, 96}, {6.20f, 3.56f, 7.76f}},
	{{-2731, -1069, 81}, {-21, 0, 64}, {7.56f, 7.35f, 6.74f}},
	{{-1805, -1587, -27}, {24, 4, 280}, {5.65f, 3.00f, 6.93f}},
	{{-4903, -1106, 82}, {30, -17, 21}, {7.39f, 7.73f, 5.14f}},
	{{-2570, -2396, 64}, {24, -33, 171}, {5.75f, 4.33f, 11.04f}},
	{{-2884, -2182, 31}, {21, -27, 112}, {7.98f, 6.25f, 7.79f}},
	{{-2412, -2318, -16}, {-15, 8, 83}, {4.10f, 3.35f, 9.23f}},
	{{-3855, -1142, 79}, {-39, -24, 241}, {4.07f, 3.66f, 11.52f}},
	{{-2145, -1791, 68}, {28, -28, 35}, {5.16f, 5.12f, 8.00f}},
	{{-1355, -1490, 98}, {-36, -9, 122}, {7.31f, 4.24f, 5.93f}},
	{{-2757, -1867, -8}, {-23, 38, 160}, {7.31f, 5.75f, 4.88f}},
	{{-4, -1246, 95}, {38, 31, 60}, {5.43f, 4.07f, 7.51f}},
	{{-4707, -1932, 98}, {-21, 26, 164}, {5.12f, 7.79f, 11.97f}},
	{{-2221, -1422, -27}, {-18, 42, 209}, {5.71f, 6.74f, 4.93f}},
	{{-2079, -1746, 78}, {-31, 41, 29}, {3.93f, 5.98f, 9.56f}},
	{{-3824, -2320, 84}, {-23, 9, 223}, {5.10f, 5.92f, 8.42f}},
	{{-326, -2194, 57}, {-24, -9, 242}, {4.50f, 4.58f, 10.14f}},
	{{-4637, -1813, 100}, {45, -38, 77}, {4.33f, 7.67f, 11.11f}},
	{{-604, -1946, -26}, {30, 18, 220}, {7.94f, 6.27f, 4.56f}},
	{{-914, -2051, 50}, {40, -33, 42}, {3.54f, 5.77f, 6.54f}},
	{{-1976, -1424, -19}, {12, -21, 176}, {7.53f, 7.23f, 5.19f}},
	{{-2882, -2085, -49}, {24, 12, 94}, {6.71f, 5.76f, 7.71f}},
	{{-4952, -2387, 82}, {36, 4, 300}, {5.91f, 3.74f, 5.46f}},
	{{-3459, -1152, 69}, {32, 36, 76}, {4.25f, 3.51f, 10.35f}},
	{{-579, -1890, 43}, {-31, 39, 311}, {7.88f, 7.05f, 11.11f}},
	{{-4876, -1395, -0}, {39, 27, 311}, {7.05f, 4.33f, 10.41f}},
	{{-4460, -1192, 79}, {-25, 28, 166}, {4.53f, 6.98f, 6.21f}},
	{{-4882, -2210, -1}, {33, 42, 100}, {6.21f, 5.00f, 11.86f}},
	{{-2319, -1091, -33}, {42, -29, 347}, {4.33f, 3.54f, 7.76f}},
	{{-1357, 765, 41}, {1, -10, 208}, {4.27f, 6.54f, 4.51f}},
	{{-372, 1169, 58}, {22, 15, 131}, {3.35f, 6.32f, 6.98f}},
	{{-3430, 1726, 58}, {-18, -17, 147}, {5.01f, 4.48f, 5.45f}},
	{{-2898, 1893, 52}, {36, 10, 108}, {5.74f, 3.00f, 6.65f}},
	{{-2851, 1244, 48}, {-3, -5, 77}, {5.37f, 7.51f, 10.47f}},
	{{-4152, 353, 27}, {12, -15, 295}, {6.76f, 6.36f, 6.18f}},
	{{-4004, 244, -13}, {-2, 31, 26}, {5.07f, 6.15f, 5.96f}},
	{{-1518, 1090, -13}, {14, -45, 270}, {6.85f, 3.53f, 7.69f}},
	{{-4121, 1924, 28}, {-40, -23, 305}, {5.28f, 7.01f, 9.51f}},
	{{-61, 1272, 93}, {35, 10, 259}, {5.52f, 7.15f, 8.61f}},
	{{-514, 1539, 21}, {-22, -23, 230}, {6.83f, 5.61f, 9.20f}},
	{{-3627, 339, -7}, {-21, -16, 194}, {3.69f, 4.16f, 9.70f}},
	{{-1468, 316, 11}, {4, -8, 74}, {5.10f, 7.52f, 8.88f}},
	{{-1522, 1742, 65}, {-11, -44, 127}, {6.77f, 7.27f, 11.65f}},
	{{-2905, 1546, 32}, {9, -25, 79}, {5.18f, 3.15f, 7.02f}},
	{{-1604, 928, -25}, {-3, -34, 224}, {3.13f, 4.97f, 8.73f}},
	{{-4864, 1357, -30}, {-3, -40, 136}, {4.06f, 4.63f, 10.21f}},
	{{-3104, 1554, 75}, {-22, -38, 7}, {5.70f, 8.00f, 7.12f}},
	{{-1749, 1606, 48}, {23, 40, 72}, {3.10f, 3.76f, 5.45f}},
	{{-1653, 1215, -17}, {18, 24, 60}, {6.04f, 6.74f, 5.36f}},
	{{-903, 1936, -34}, {-43, -17, 244}, {7.79f, 4.98f, 9.86f}},
	{{-4620, 1443, 44}, {-36, 25, 306}, {6.00f, 3.61f, 11.88f}},
	{{-1087, 825, 14}, {-12, 1, 123}, {7.25f, 7.11f, 5.29f}},
	{{-196, 1344, 74}, {19, -6, 264}, {7.83f, 4.35f, 10.56f}},
	{{-2309, 1070, 15}, {21, -21, 307}, {7.15f, 3.43f, 11.11f}},
	{{-3781, 1036, 42}, {-11, -42, 306}, {3.91f, 4.06f, 10.48f}},
	{{-3298, 1785, 55}, {-20, -44, 341}, {3.43f, 6.60f, 8.16f}},
	{{-1209, 1443, 47}, {-1, 26, 33}, {4.11f, 6.46f, 6.80f}},
	{{-2092, 1052, 30}, {-7, 22, 119}, {6.51f, 4.35f, 6.39f}},
	{{-4397, 547, -32}, {3, 24, 67}, {4.08f, 5.42f, 9.93f}},
	{{-117, 1144, -8}, {-36, -28, 82}, {3.90f, 3.07f, 8.51f}},
	{{-3628, 1954, 33}, {18, -34, 313}, {5.45f, 7.36f, 8.81f}},
	{{-2653, 993, -22}, {-40, 40, 172}, {7.11f, 5.00f, 5.06f}},
	{{-1853, 296, -28}, {6, -18, 358}, {3.59f, 6.82f, 9.05f}},
	{{-1046, 606, 28}, {-4, -5, 310}, {7.95f, 4.53f, 9.16f}},
	{{-1952, 1532, 92}, {-26, -26, 238}, {3.79f, 3.87f, 5.06f}},
	{{-4987, 1011, 39}, {-19, -24, 255}, {6.51f, 5.27f, 9.66f}},
	{{-380, 1618, 44}, {15, 39, 153}, {5.72f, 6.24f, 11.31f}},
	{{-867, 329, -25}, {-17, 22, 205}, {4.44f, 3.62f, 9.67f}},
	{{-1501, 1897, 25}, {-1, -38, 14}, {5.16f, 4.61f, 6.38f}},
	{{728, 2771, 25}, {7, 41, 360}, {11.72f, 7.70f, 8.10f}},
	{{2391, -177, -2}, {37, -29, 211}, {11.35f, 9.92f, 8.87f}},
	{{1370, -1000, 1}, {32, -15, 250}, {7.88f, 14.45f, 19.70f}},
	{{1875, -271, -53}, {-16, 42, 146}, {10.15f, 14.88f, 17.36f}},
	{{1856, -521, -72}, {-12, 23, 225}, {12.60f, 7.04f, 15.74f}},
	{{2819, -371, 5}, {-34, 43, 219}, {7.39f, 6.58f, 15.76f}},
	{{1881, -2441, 49}, {37, -3, 42}, {13.32f, 9.98f, 18.25f}},
	{{1772, -1359, 25}, {43, -23, 198}, {8.84f, 14.22f, 15.12f}},
	{{2698, 2184, -59}, {26, -8, 336}, {10.08f, 13.21f, 11.74f}},
	{{1246, 522, 50}, {-1, -32, 194}, {8.45f, 10.52f, 15.65f}},
	{{1638, -1069, -72}, {18, 6, 84}, {12.76f, 5.44f, 18.67f}},
	{{2263, 1868, -42}, {15, 29, 353}, {9.95f, 5.37f, 15.03f}},
	{{1975, 2218, 31}, {-5, 2, 164}, {12.22f, 9.10f, 17.32f}},
	{{886, -183, 45}, {-15, 17, 234}, {13.52f, 13.52f, 20.39f}},
	{{1450, -1100, 8}, {23, 34, 13}, {5.68f, 11.31f, 21.31f}},
	{{2994, 1481, -35}, {-36, 12, 314}, {9.44f, 11.94f, 21.05f}},
	{{615, 1777, -56}, {-11, -32, 191}, {10.66f, 12.93f, 10.05f}},
	{{697, 2225, -7}, {-23, 37, 52}, {9.61f, 7.54f, 11.33f}},
	{{523, 1828, 35}, {16, -31, 159}, {8.46f, 10.88f, 17.08f}},
	{{1561, -1499, 27}, {-27, -10, 174}, {7.37f, 10.72f, 16.12f}},
	{{2982, -1229, 47}, {14, -20, 204}, {11.86f, 12.45f, 8.24f}},
	{{2016, -20, 36}, {-19, 27, 219}, {8.52f, 11.37f, 16.81f}},
	{{2194, 1326, -1}, {30, 12, 325}, {11.46f, 8.09f, 14.11f}},
	{{1949, 1394, -86}, {-18, 22, 63}, {6.32f, 10.39f, 22.07f}},
	{{1827, 2481, 25}, {-22, 29, 173}, {13.06f, 12.47f, 12.58f}},
	{{788, 2777, -79}, {42, 32, 261}, {14.80f, 14.67f, 19.57f}},
	{{1414, 1744, -98}, {3, -4, 242}, {11.72f, 10.85f, 19.84f}},
	{{2851, -2350, -65}, {-43, 35, 202}, {14.15f, 7.21f, 8.45f}},
	{{2560, 2456, -55}, {-8, -32, 341}, {8.04f, 9.93f, 8.96f}},
	{{2718, -2186, -32}, {15, 22, 341}, {9.19f, 12.42f, 9.82f}},
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

	// The island ground sits above this broad, engine-native water surface;
	// its material supplies moving water without adding a plugin or simulation.
	OceanSurface = AddShape(TEXT("CoastalOcean"), PLANE_MESH,
		FVector(0.0f, 0.0f, -24.0f), FRotator::ZeroRotator, FVector(1400.0f, 1400.0f, 1.0f), false);
	if (OceanSurface)
	{
		OceanSurface->SetMaterial(0, LoadObject<UMaterialInterface>(nullptr, OCEAN_MATERIAL));
		OceanSurface->SetCastShadow(false);
	}

	AnnexHeroDetails = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AnnexHeroDetails"));
	AnnexHeroDetails->SetupAttachment(SceneRoot);
	AnnexHeroDetails->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,
		TEXT("/Game/BlackBeacon/Art/Lighthouse/Meshes/SM_BB_LH_AnnexDetails.SM_BB_LH_AnnexDetails")));
	AnnexHeroDetails->SetRelativeLocation(FVector(1560.0f, 1120.0f, 0.0f));
	AnnexHeroDetails->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AnnexHeroDetails->SetCanEverAffectNavigation(false);
	AnnexHeroDetails->SetCastShadow(true);

	// The low shelf stays a broad shore form. A single engine-sample boulder
	// mesh gives the instanced field a faceted coastal silhouette.
	const FCoastShape& Shelf = ROCKS[0];
	RockSurfaces.Add(AddShape(
		TEXT("CoastShelf"), CUBE_MESH,
		Shelf.Location, Shelf.Rotation, Shelf.Scale, false));

	RockField = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("CoastRockField"));
	RockField->SetupAttachment(SceneRoot);
	RockField->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, ROCK_MESH));
	RockField->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	RockField->SetCollisionResponseToAllChannels(ECR_Block);
	RockField->SetGenerateOverlapEvents(false);
	RockField->SetCanEverAffectNavigation(false);
	RockField->SetCastShadow(true);
	RockSurfaces.Add(RockField);
	for (int32 Index = 1; Index < UE_ARRAY_COUNT(ROCKS); ++Index)
	{
		const FCoastShape& Shape = ROCKS[Index];
		const float Angle = static_cast<float>(Index) * 2.39996323f;
		const FVector OffsetA(
			FMath::Cos(Angle) * Shape.Scale.X * 24.0f,
			FMath::Sin(Angle) * Shape.Scale.Y * 24.0f,
			Shape.Scale.Z * 8.0f);
		const FVector OffsetB(
			-FMath::Sin(Angle) * Shape.Scale.X * 21.0f,
			FMath::Cos(Angle) * Shape.Scale.Y * 21.0f,
			-Shape.Scale.Z * 10.0f);

		// Overlapping lobes break the source mesh silhouette without extra components.
		RockField->AddInstance(FTransform(Shape.Rotation, Shape.Location, Shape.Scale * (0.68f * ROCK_SAMPLE_SCALE)));
		RockField->AddInstance(FTransform(Shape.Rotation, Shape.Location + OffsetA,
			Shape.Scale * FVector(0.43f, 0.50f, 0.45f) * ROCK_SAMPLE_SCALE));
		RockField->AddInstance(FTransform(Shape.Rotation, Shape.Location + OffsetB,
			Shape.Scale * FVector(0.46f, 0.41f, 0.48f) * ROCK_SAMPLE_SCALE));
	}

	for (int32 Index = 0; Index < UE_ARRAY_COUNT(PATH_STONES); ++Index)
	{
		const FCoastShape& Shape = PATH_STONES[Index];
		PathSurfaces.Add(AddShape(
			*FString::Printf(TEXT("Path_%02d"), Index), CYLINDER_MESH,
			Shape.Location, Shape.Rotation, Shape.Scale, false));
	}

	for (int32 Index = 0; Index < UE_ARRAY_COUNT(WRECKAGE); ++Index)
	{
		const FCoastShape& Shape = WRECKAGE[Index];
		WreckSurfaces.Add(AddShape(
			*FString::Printf(TEXT("Wreck_%02d"), Index), CUBE_MESH,
			Shape.Location, Shape.Rotation, Shape.Scale, true));
	}

	// A small weather-beaten engine annex replaces the interaction cube with
	// a traversable shell. The west wall leaves a centered doorway to the
	// generator so the existing approach and interaction line stay clear.
	const auto AddAnnexPart = [this](const TCHAR* Name, const FVector& Location,
		const FRotator& Rotation, const FVector& Scale)
	{
		AnnexSurfaces.Add(AddShape(Name, CUBE_MESH, Location, Rotation, Scale, true));
	};
	AddAnnexPart(TEXT("AnnexFloor"), FVector(1560.0f, 1120.0f, -8.0f), FRotator::ZeroRotator, FVector(3.15f, 4.15f, 0.16f));
	AddAnnexPart(TEXT("AnnexWestLeft"), FVector(1400.0f, 940.0f, 140.0f), FRotator::ZeroRotator, FVector(0.18f, 1.2f, 2.8f));
	AddAnnexPart(TEXT("AnnexWestRight"), FVector(1400.0f, 1300.0f, 140.0f), FRotator::ZeroRotator, FVector(0.18f, 1.2f, 2.8f));
	AddAnnexPart(TEXT("AnnexEastWall"), FVector(1720.0f, 1120.0f, 140.0f), FRotator::ZeroRotator, FVector(0.18f, 4.2f, 2.8f));
	AddAnnexPart(TEXT("AnnexNorthWall"), FVector(1560.0f, 1330.0f, 140.0f), FRotator::ZeroRotator, FVector(3.2f, 0.18f, 2.8f));
	AddAnnexPart(TEXT("AnnexSouthWall"), FVector(1560.0f, 910.0f, 140.0f), FRotator::ZeroRotator, FVector(3.2f, 0.18f, 2.8f));
	AddAnnexPart(TEXT("AnnexRoofWest"), FVector(1480.0f, 1120.0f, 292.0f), FRotator(22.0f, 0.0f, 0.0f), FVector(1.72f, 2.2f, 0.18f));
	AddAnnexPart(TEXT("AnnexRoofEast"), FVector(1640.0f, 1120.0f, 292.0f), FRotator(-22.0f, 0.0f, 0.0f), FVector(1.72f, 2.2f, 0.18f));
}

void ABBCoastalEnvironment::BeginPlay()
{
	Super::BeginPlay();

	UMaterialInterface* const CoastMaterial = LoadObject<UMaterialInterface>(nullptr, COAST_MATERIAL);
	UMaterialInterface* const BasaltMaterial = LoadObject<UMaterialInterface>(nullptr, BASALT_MATERIAL);
	auto ApplyWetMaterial = [this](
		const TArray<TObjectPtr<UStaticMeshComponent>>& Components,
		UMaterialInterface* BaseMaterial,
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

	ApplyWetMaterial(RockSurfaces, BasaltMaterial, FLinearColor(0.78f, 0.84f, 0.88f), 0.82f);
	ApplyWetMaterial(PathSurfaces, CoastMaterial, FLinearColor(0.050f, 0.065f, 0.075f), 0.52f);
	ApplyWetMaterial(WreckSurfaces, CoastMaterial, FLinearColor(0.075f, 0.042f, 0.022f), 0.48f);
	ApplyWetMaterial(AnnexSurfaces, CoastMaterial, FLinearColor(0.075f, 0.055f, 0.040f), 0.72f);
	DressLighthouseGreybox();
	BuildGeneratorMachinery();
	BuildRevealedRuin();
}

void ABBCoastalEnvironment::DressLighthouseGreybox()
{
	UMaterialInterface* const StairMetal = LoadObject<UMaterialInterface>(nullptr,
		TEXT("/Game/BlackBeacon/Art/Lighthouse/Materials/M_LH_DarkIron.M_LH_DarkIron"));
	constexpr float TowerFloorHeightCm = 520.0f;
	constexpr float StairRadiusByFloor[] = {190.0f, 155.0f, 130.0f};
	constexpr float StairDepthByFloor[] = {130.0f, 110.0f, 90.0f};
	UWorld* const World = GetWorld();
	if (!World)
	{
		return;
	}
	const FVector AnnexOffset = GENERATOR_ANNEX_HERO_CENTER - GENERATOR_ANNEX_OLD_CENTER;
	for (UStaticMeshComponent* const Part : AnnexSurfaces)
	{
		if (Part)
		{
			Part->SetWorldLocation(Part->GetComponentLocation() + AnnexOffset);
		}
	}
	if (AnnexHeroDetails)
	{
		AnnexHeroDetails->SetWorldLocation(AnnexHeroDetails->GetComponentLocation() + AnnexOffset);
	}

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* const Actor = *It;
		if (!Actor)
		{
			continue;
		}
		if (Actor->ActorHasTag(TEXT("BB_GeneratorShed")) || Actor->ActorHasTag(TEXT("BB_Generator")))
		{
			const FVector Location = Actor->GetActorLocation();
			if (FVector2D(Location - GENERATOR_ANNEX_OLD_CENTER).SizeSquared() < 1.0f)
			{
				Actor->AddActorWorldOffset(AnnexOffset);
			}
		}
		else if (TArray<UBBObjectiveTriggerComponent*> Triggers;
			Actor->GetComponents<UBBObjectiveTriggerComponent>(Triggers), Triggers.Num() > 0)
		{
			for (UBBObjectiveTriggerComponent* const Trigger : Triggers)
			{
				if (Trigger && Trigger->ObjectiveId == TEXT("BB_OBJ_FIND_GENERATOR")
					&& FVector2D(Actor->GetActorLocation() - GENERATOR_ANNEX_OLD_CENTER).SizeSquared() < 1.0f)
				{
					Actor->AddActorWorldOffset(AnnexOffset);
				}
			}
		}
		else if (Actor->ActorHasTag(TEXT("BB_TowerWall")) || Actor->ActorHasTag(TEXT("BB_TowerFloor")))
		{
			for (UStaticMeshComponent* const Mesh : TInlineComponentArray<UStaticMeshComponent*>(Actor))
			{
				Mesh->SetVisibility(false);
			}
		}
		else if (Actor->ActorHasTag(TEXT("BB_StairCore")))
		{
			if (UStaticMeshComponent* const CoreMesh = Actor->FindComponentByClass<UStaticMeshComponent>())
			{
				CoreMesh->SetWorldScale3D(FVector(0.9f, 0.9f, CoreMesh->GetComponentScale().Z));
			}
		}
		else if (Actor->ActorHasTag(TEXT("BB_StairEntryLanding")))
		{
			Actor->SetActorLocation(FVector(310.0f, 0.0f, Actor->GetActorLocation().Z));
			if (UStaticMeshComponent* const EntryMesh = Actor->FindComponentByClass<UStaticMeshComponent>())
			{
				EntryMesh->SetWorldScale3D(FVector(1.7f, 1.0f, EntryMesh->GetComponentScale().Z));
			}
		}
		else if (Actor->ActorHasTag(TEXT("BB_LanternFloor")))
		{
			Actor->SetActorLocation(FVector(120.0f, -35.0f, Actor->GetActorLocation().Z));
			if (UStaticMeshComponent* const LanternFloorMesh = Actor->FindComponentByClass<UStaticMeshComponent>())
			{
				LanternFloorMesh->SetWorldScale3D(FVector(1.1f, 1.2f, LanternFloorMesh->GetComponentScale().Z));
			}
		}
		else if (Actor->ActorHasTag(TEXT("BB_StairStep"))
			|| Actor->ActorHasTag(TEXT("BB_StairGuard")))
		{
			if (UStaticMeshComponent* const Mesh = Actor->FindComponentByClass<UStaticMeshComponent>())
			{
				if (StairMetal)
				{
					Mesh->SetMaterial(0, StairMetal);
				}

				// The saved map contains the original constant-radius stair actors.
				// Keep their rise, rotation, and collision while tucking each upper
				// flight inside the tapered hero tower.
				const FVector Location = Actor->GetActorLocation();
				const bool bIsGuard = Actor->ActorHasTag(TEXT("BB_StairGuard"));
				const float FlightZ = Location.Z - (bIsGuard ? 48.0f : 0.0f);
				const int32 Floor = FMath::Clamp(
					FMath::FloorToInt(FlightZ / TowerFloorHeightCm), 0, 2);
				const float Radius = StairRadiusByFloor[Floor];
				const float Depth = StairDepthByFloor[Floor];
				const FRotator Rotation = Actor->GetActorRotation();
				const float Angle = FMath::DegreesToRadians(Rotation.Yaw);
				const float TargetRadius = bIsGuard ? Radius + Depth * 0.5f - 15.0f : Radius;
				Actor->SetActorLocation(FVector(
					FMath::Cos(Angle) * TargetRadius,
					FMath::Sin(Angle) * TargetRadius,
					Location.Z));
				if (!bIsGuard)
				{
					FVector MeshScale = Mesh->GetComponentScale();
					MeshScale.X = Depth / 100.0f;
					Mesh->SetWorldScale3D(MeshScale);
				}
			}
		}
	}
}

void ABBCoastalEnvironment::BuildGeneratorMachinery()
{
	UWorld* const World = GetWorld();
	if (!World)
	{
		return;
	}

	AActor* Generator = nullptr;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		if (It->ActorHasTag(TEXT("BB_Generator")))
		{
			Generator = *It;
			break;
		}
	}
	if (!Generator || !Generator->GetRootComponent())
	{
		return;
	}
	GeneratorComponent = Generator->FindComponentByClass<UBBGeneratorComponent>();

	UStaticMesh* const Cube = LoadObject<UStaticMesh>(nullptr, CUBE_MESH);
	UStaticMesh* const Cylinder = LoadObject<UStaticMesh>(nullptr, CYLINDER_MESH);
	UMaterialInterface* const MaterialBase = LoadObject<UMaterialInterface>(nullptr, COAST_MATERIAL);
	if (!Cube || !Cylinder || !MaterialBase)
	{
		return;
	}

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		if (!It->ActorHasTag(TEXT("BB_GeneratorShed")))
		{
			continue;
		}

		for (UStaticMeshComponent* const Mesh : TInlineComponentArray<UStaticMeshComponent*>(*It))
		{
			if (Mesh && Mesh->GetName() == TEXT("Mesh"))
			{
				// The procedural box is replaced by the traversable annex shell built above.
				Mesh->SetVisibility(false, false);
			}
		}
		break;
	}

	const auto MakeInstanceField = [Generator, MaterialBase](
		const TCHAR* Name, UStaticMesh* Mesh, const FLinearColor& Tint, float Roughness)
	{
		UInstancedStaticMeshComponent* const Field = NewObject<UInstancedStaticMeshComponent>(Generator, Name);
		Generator->AddInstanceComponent(Field);
		Field->SetupAttachment(Generator->GetRootComponent());
		Field->SetStaticMesh(Mesh);
		Field->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Field->SetCastShadow(false);
		Field->SetCanEverAffectNavigation(false);
		UMaterialInstanceDynamic* const Material = UMaterialInstanceDynamic::Create(MaterialBase, Field);
		Material->SetVectorParameterValue(TEXT("BaseColor"), Tint);
		Material->SetScalarParameterValue(TEXT("Roughness"), Roughness);
		Field->SetMaterial(0, Material);
		return Field;
	};
	const auto MakeFitting = [Generator, MaterialBase](
		const TCHAR* Name, UStaticMesh* Mesh, const FVector& Location,
		const FRotator& Rotation, const FVector& Scale,
		const FLinearColor& Tint, float Roughness)
	{
		UStaticMeshComponent* const Fitting = NewObject<UStaticMeshComponent>(Generator, Name);
		Generator->AddInstanceComponent(Fitting);
		Fitting->SetupAttachment(Generator->GetRootComponent());
		Fitting->SetStaticMesh(Mesh);
		Fitting->SetRelativeTransform(FTransform(Rotation, Location, Scale));
		Fitting->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Fitting->SetCastShadow(false);
		Fitting->SetCanEverAffectNavigation(false);
		UMaterialInstanceDynamic* const Material = UMaterialInstanceDynamic::Create(MaterialBase, Fitting);
		Material->SetVectorParameterValue(TEXT("BaseColor"), Tint);
		Material->SetScalarParameterValue(TEXT("Roughness"), Roughness);
		Fitting->SetMaterial(0, Material);
		Fitting->RegisterComponent();
		return Fitting;
	};

	TArray<UStaticMeshComponent*> ExistingMeshes;
	Generator->GetComponents<UStaticMeshComponent>(ExistingMeshes);
	for (UStaticMeshComponent* const Mesh : ExistingMeshes)
	{
		if (Mesh && Mesh->GetName() == TEXT("Mesh"))
		{
			// Keep the original interaction collider, but replace its cube silhouette with visible machine parts.
			Mesh->SetVisibility(false, false);
			break;
		}
	}

	const FLinearColor IronTint(0.12f, 0.14f, 0.15f);
	const FLinearColor CopperTint(0.24f, 0.105f, 0.035f);
	const FLinearColor DialTint(0.38f, 0.32f, 0.22f);
	MakeFitting(TEXT("GeneratorBody"), Cube, FVector(0.0f, 0.0f, -4.0f),
		FRotator::ZeroRotator, FVector(1.08f, 0.78f, 0.62f), IronTint, 0.72f);
	MakeFitting(TEXT("GeneratorCylinderHead"), Cube, FVector(12.0f, 0.0f, 58.0f),
		FRotator::ZeroRotator, FVector(0.72f, 0.58f, 0.20f), CopperTint, 0.64f);
	UInstancedStaticMeshComponent* const SkidAndRibs = MakeInstanceField(
		TEXT("GeneratorSkidAndRibs"), Cube, IronTint, 0.76f);
	SkidAndRibs->AddInstance(FTransform(FRotator::ZeroRotator,
		FVector(0.0f, 0.0f, -74.0f), FVector(1.82f, 1.52f, 0.16f)));
	for (const float SideY : {-52.0f, 52.0f})
	{
		SkidAndRibs->AddInstance(FTransform(FRotator::ZeroRotator,
			FVector(-18.0f, SideY, 0.0f), FVector(1.22f, 0.12f, 0.92f)));
	}
	SkidAndRibs->RegisterComponent();

	GeneratorFlywheelPivot = NewObject<USceneComponent>(Generator, TEXT("GeneratorFlywheelPivot"));
	Generator->AddInstanceComponent(GeneratorFlywheelPivot);
	GeneratorFlywheelPivot->SetupAttachment(Generator->GetRootComponent());
	GeneratorFlywheelPivot->SetRelativeLocation(FVector(-116.0f, 0.0f, -4.0f));
	GeneratorFlywheelPivot->SetCanEverAffectNavigation(false);
	GeneratorFlywheelPivot->RegisterComponent();

	UStaticMeshComponent* const Flywheel = MakeFitting(
		TEXT("GeneratorFlywheel"), Cylinder, FVector(-116.0f, 0.0f, -4.0f),
		FRotator(90.0f, 0.0f, 0.0f), FVector(0.72f, 0.72f, 0.10f), IronTint, 0.52f);
	Flywheel->AttachToComponent(GeneratorFlywheelPivot, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	Flywheel->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	UInstancedStaticMeshComponent* const FlywheelSpokes = MakeInstanceField(
		TEXT("GeneratorFlywheelSpokes"), Cube, CopperTint, 0.50f);
	FlywheelSpokes->AttachToComponent(GeneratorFlywheelPivot, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	for (int32 Spoke = 0; Spoke < 4; ++Spoke)
	{
		FlywheelSpokes->AddInstance(FTransform(FRotator(0.0f, 0.0f, Spoke * 45.0f),
			FVector(-13.0f, 0.0f, 0.0f), FVector(0.055f, 0.57f, 0.045f)));
	}
	FlywheelSpokes->RegisterComponent();
	UStaticMeshComponent* const FlywheelHub = MakeFitting(TEXT("GeneratorFlywheelHub"), Cylinder, FVector(-135.0f, 0.0f, -4.0f),
		FRotator(90.0f, 0.0f, 0.0f), FVector(0.23f, 0.23f, 0.08f), CopperTint, 0.44f);
	FlywheelHub->AttachToComponent(GeneratorFlywheelPivot, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	FlywheelHub->SetRelativeLocation(FVector(-19.0f, 0.0f, 0.0f));
	FlywheelHub->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	MakeFitting(TEXT("GeneratorGauge"), Cylinder, FVector(-116.0f, 45.0f, 42.0f),
		FRotator(90.0f, 0.0f, 0.0f), FVector(0.19f, 0.19f, 0.10f), DialTint, 0.66f);
	MakeFitting(TEXT("GeneratorRegulatorA"), Cylinder, FVector(34.0f, -43.0f, 4.0f),
		FRotator::ZeroRotator, FVector(0.16f, 0.16f, 0.68f), CopperTint, 0.48f);
	MakeFitting(TEXT("GeneratorRegulatorB"), Cylinder, FVector(34.0f, 43.0f, 4.0f),
		FRotator::ZeroRotator, FVector(0.16f, 0.16f, 0.68f), CopperTint, 0.48f);

	UPointLightComponent* const ShedLight = NewObject<UPointLightComponent>(this, TEXT("GeneratorShedLight"));
	AddInstanceComponent(ShedLight);
	ShedLight->SetupAttachment(SceneRoot);
	ShedLight->SetMobility(EComponentMobility::Movable);
	ShedLight->SetRelativeLocation(GENERATOR_ANNEX_HERO_CENTER + FVector(0.0f, 0.0f, 238.0f));
	ShedLight->IntensityUnits = ELightUnits::Lumens;
	ShedLight->SetIntensity(GENERATOR_SHED_LIGHT_LUMENS);
	ShedLight->SetLightColor(FLinearColor(1.0f, 0.66f, 0.38f));
	ShedLight->SetAttenuationRadius(620.0f);
	ShedLight->SetCastShadows(false);
	ShedLight->RegisterComponent();

	if (GeneratorComponent)
	{
		GeneratorComponent->OnGeneratorRunningChanged.AddUObject(this, &ABBCoastalEnvironment::HandleGeneratorRunningChanged);
		if (GeneratorComponent->IsRunning())
		{
			HandleGeneratorRunningChanged(true);
		}
	}
}

void ABBCoastalEnvironment::HandleGeneratorRunningChanged(bool bRunning)
{
	if (!GeneratorFlywheelPivot || !GetWorld())
	{
		return;
	}
	if (bRunning)
	{
		GetWorldTimerManager().SetTimer(GeneratorFlywheelTimer, this,
			&ABBCoastalEnvironment::AdvanceGeneratorFlywheel, 0.05f, true, 0.0f);
	}
	else if (GeneratorFlywheelSpeedDegrees > 1.0f)
	{
		// Keep the visual motor alive briefly so the heavy wheel can coast down.
		GetWorldTimerManager().SetTimer(GeneratorFlywheelTimer, this,
			&ABBCoastalEnvironment::AdvanceGeneratorFlywheel, 0.05f, true, 0.0f);
	}
	else
	{
		GetWorldTimerManager().ClearTimer(GeneratorFlywheelTimer);
	}
}

void ABBCoastalEnvironment::AdvanceGeneratorFlywheel()
{
	if (!GeneratorFlywheelPivot || !GeneratorComponent)
	{
		return;
	}
	const float DeltaSeconds = 0.05f;
	const float TargetSpeed = GeneratorComponent->IsRunning()
		? 300.0f * GeneratorComponent->GetSpinUpProgress() : 0.0f;
	GeneratorFlywheelSpeedDegrees = FMath::FInterpTo(GeneratorFlywheelSpeedDegrees, TargetSpeed, DeltaSeconds, 2.5f);
	GeneratorFlywheelPivot->AddLocalRotation(FRotator(0.0f, 0.0f, GeneratorFlywheelSpeedDegrees * DeltaSeconds));
	if (!GeneratorComponent->IsRunning() && GeneratorFlywheelSpeedDegrees <= 1.0f)
	{
		GeneratorFlywheelSpeedDegrees = 0.0f;
		GetWorldTimerManager().ClearTimer(GeneratorFlywheelTimer);
	}
}

void ABBCoastalEnvironment::BuildRevealedRuin()
{
	UWorld* const World = GetWorld();
	if (!World)
	{
		return;
	}

	AActor* Ruin = nullptr;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		if (It->ActorHasTag(TEXT("BB_Anomaly")))
		{
			Ruin = *It;
			break;
		}
	}
	if (!Ruin || !Ruin->GetRootComponent())
	{
		return;
	}

	// Keep the old greybox mesh out of the reveal. These stone fragments are
	// attached to the same actor, so the real BeamReveal state owns visibility.
	for (UStaticMeshComponent* const Mesh : TInlineComponentArray<UStaticMeshComponent*>(Ruin))
	{
		if (Mesh && Mesh->GetAttachParent() == Ruin->GetRootComponent())
		{
			Mesh->SetVisibility(false, false);
		}
	}

	UMaterialInterface* const StoneMaterial = LoadObject<UMaterialInterface>(nullptr, BASALT_MATERIAL);
	UTexture2D* const HullAlbedo = LoadObject<UTexture2D>(nullptr, WRECK_HULL_TEXTURE);
	if (!StoneMaterial)
	{
		return;
	}

	// This CC0 hull section provides authored curvature and frame detail; each
	// imported material mesh remains a separate beam-reveal piece.
	const TCHAR* const WreckMeshPaths[] = {
		TEXT("/Game/BlackBeacon/Meshes/blackbeacon_wreck_hull/StaticMeshes/shipwreck-hull-section_0.shipwreck-hull-section_0"),
		TEXT("/Game/BlackBeacon/Meshes/blackbeacon_wreck_hull/StaticMeshes/shipwreck-hull-section_1.shipwreck-hull-section_1"),
		TEXT("/Game/BlackBeacon/Meshes/blackbeacon_wreck_hull/StaticMeshes/shipwreck-hull-section_2.shipwreck-hull-section_2"),
		TEXT("/Game/BlackBeacon/Meshes/blackbeacon_wreck_hull/StaticMeshes/shipwreck-hull-section_3.shipwreck-hull-section_3")
	};
	UStaticMesh* const Cube = LoadObject<UStaticMesh>(nullptr, CUBE_MESH);
	float WreckYawOffset = 0.0f;
	for (TActorIterator<ABBLighthouseController> It(World); It; ++It)
	{
		if (It->BeamComponent)
		{
			const FVector ToWreck = Ruin->GetActorLocation() - It->BeamComponent->GetComponentLocation();
			WreckYawOffset = ToWreck.Rotation().Yaw + 90.0f - Ruin->GetActorRotation().Yaw;
			break;
		}
	}
	const FCoastShape ExposedWreckFrame[] = {
		{{-155.0f,   0.0f,  94.0f}, {0.0f, 0.0f,  12.0f}, {0.16f, 0.92f, 0.20f}},
		{{ -65.0f,   0.0f,  98.0f}, {0.0f, 0.0f, -10.0f}, {0.14f, 0.98f, 0.18f}},
		{{  35.0f,   0.0f,  94.0f}, {0.0f, 0.0f,  16.0f}, {0.16f, 0.91f, 0.19f}},
		{{ 130.0f,   0.0f,  82.0f}, {0.0f, 0.0f, -18.0f}, {0.15f, 0.78f, 0.18f}},
		{{-120.0f, -78.0f, 112.0f}, {0.0f, 0.0f,  -8.0f}, {1.20f, 0.14f, 0.14f}},
		{{  -5.0f,  78.0f, 116.0f}, {0.0f, 0.0f,   9.0f}, {1.35f, 0.14f, 0.14f}},
		{{-105.0f,   0.0f, 270.0f}, {7.0f, 0.0f,  -5.0f}, {0.18f, 0.20f, 3.10f}},
		{{-100.0f,  10.0f, 430.0f}, {0.0f, 0.0f,  24.0f}, {1.45f, 0.14f, 0.14f}},
		{{ -55.0f, -10.0f, 285.0f}, {28.0f, 0.0f, 0.0f}, {1.70f, 0.12f, 0.12f}},
		{{ 155.0f,  35.0f, 175.0f}, {-8.0f, 0.0f, 27.0f}, {1.00f, 0.13f, 0.13f}}
	};

	int32 CreatedWreckParts = 0;
	for (int32 Index = 0; Index < UE_ARRAY_COUNT(WreckMeshPaths); ++Index)
	{
		UStaticMesh* const WreckMesh = LoadObject<UStaticMesh>(nullptr, WreckMeshPaths[Index]);
		if (!WreckMesh)
		{
			continue;
		}

		UStaticMeshComponent* const Mesh = NewObject<UStaticMeshComponent>(
			Ruin, *FString::Printf(TEXT("RevealedRuinPart_%02d"), Index));
		Ruin->AddInstanceComponent(Mesh);
		Mesh->SetupAttachment(Ruin->GetRootComponent());
		Mesh->SetStaticMesh(WreckMesh);
		Mesh->SetRelativeTransform(FTransform(FRotator(7.0f, -14.0f + WreckYawOffset, 11.0f),
			FVector::ZeroVector, FVector(4.2f)));
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Mesh->SetCastShadow(true);
		Mesh->SetMaterial(0, StoneMaterial);
		Mesh->ComponentTags.Add(TEXT("BB_BeamRevealPart"));
		Mesh->RegisterComponent();
		UMaterialInstanceDynamic* const Material = Mesh->CreateDynamicMaterialInstance(0);
		if (Material)
		{
			if (HullAlbedo)
			{
				Material->SetTextureParameterValue(TEXT("RockAlbedo"), HullAlbedo);
			}
			Material->SetScalarParameterValue(TEXT("Roughness"), 0.72f);
		}
		++CreatedWreckParts;
	}
	if (Cube)
	{
		for (int32 Index = 0; Index < UE_ARRAY_COUNT(ExposedWreckFrame); ++Index)
		{
			const FCoastShape& Part = ExposedWreckFrame[Index];
			UStaticMeshComponent* const Mesh = NewObject<UStaticMeshComponent>(
				Ruin, *FString::Printf(TEXT("RevealedRuinFrame_%02d"), Index));
			Ruin->AddInstanceComponent(Mesh);
			Mesh->SetupAttachment(Ruin->GetRootComponent());
			Mesh->SetStaticMesh(Cube);
			Mesh->SetRelativeTransform(FTransform(FRotator(Part.Rotation.Pitch,
				Part.Rotation.Yaw + WreckYawOffset, Part.Rotation.Roll),
				Part.Location * 6.0f, Part.Scale * 6.0f));
			Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Mesh->SetCastShadow(true);
			Mesh->SetMaterial(0, StoneMaterial);
			Mesh->ComponentTags.Add(TEXT("BB_BeamRevealPart"));
			Mesh->RegisterComponent();
			UMaterialInstanceDynamic* const Material = Mesh->CreateDynamicMaterialInstance(0);
			if (Material)
			{
				Material->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.54f, 0.61f, 0.67f));
				Material->SetScalarParameterValue(TEXT("Roughness"), 0.84f);
			}
			++CreatedWreckParts;
		}
	}
	if (CreatedWreckParts == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("BBBeamReveal: failed to load any imported wreck meshes."));
	}

	if (UBBBeamRevealComponent* const Reveal = Ruin->FindComponentByClass<UBBBeamRevealComponent>())
	{
		Reveal->EnableTaggedPartReveal();
	}
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
