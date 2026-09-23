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

	for (int32 Index = 0; Index < UE_ARRAY_COUNT(ROCKS); ++Index)
	{
		const FCoastShape& Shape = ROCKS[Index];
		RockSurfaces.Add(AddShape(
			*FString::Printf(TEXT("Rock_%02d"), Index),
			CUBE_MESH,
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
