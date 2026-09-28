#include "BBEnvironmentKit.h"

#include "BlackBeacon/Lighthouse/BBLighthouseController.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "Materials/MaterialInterface.h"

namespace BlackBeacon::EnvironmentKit
{
namespace
{
    struct FVisualPlacement
    {
        const TCHAR* Asset;
        FVector Foot;
        float Yaw;
        float WidthCm;
    };

    // Foot positions are authored outside the approach, doorway, annex entry and
    // playable stairs. Width normalizes FBX units without stretching scan strata.
    const FVisualPlacement PLACEMENTS[] =
    {
        {TEXT("coast_rocks_05"), {490, 500, -35}, 30, 340},
        {TEXT("coast_rocks_05"), {90, 660, -30}, 95, 370},
        {TEXT("coast_rocks_05"), {-360, 570, -30}, 150, 320},
        {TEXT("coast_rocks_05"), {-680, 300, -40}, 215, 370},
        {TEXT("coast_rocks_05"), {-610, -330, -35}, 255, 340},
        {TEXT("coast_rocks_05"), {560, -480, -40}, 320, 320},
        {TEXT("rock_face_02"), {2360, -900, -400}, 130, 500},
        {TEXT("rock_face_02"), {2280, 1550, -405}, 65, 470},
        {TEXT("rock_face_02"), {400, -2070, -410}, 185, 490},
        {TEXT("coast_rocks_05"), {2360, -690, -230}, 120, 430},
        {TEXT("coast_rocks_05"), {520, -2100, -235}, 195, 420},
        {TEXT("barrel_03"), {-133, -486, 20}, 12, 64},
        {TEXT("barrel_03"), {-61, -486, 20}, -19, 64},
        {TEXT("barrel_03"), {-261, -548, 0}, 22, 64},
    };
}

void Assemble(ABBLighthouseController& Lighthouse)
{
    UMaterialInterface* Exterior = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Game/BlackBeacon/EnvironmentKitV1/Materials/MI_EK_ExteriorPlaster"));
    UMaterialInterface* Interior = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Game/BlackBeacon/EnvironmentKitV1/Materials/MI_EK_InteriorPlaster"));
    if (!Exterior || !Interior)
    {
        UE_LOG(LogTemp, Error, TEXT("Environment kit wall materials are missing; retain existing visuals."));
        return;
    }
    for (TActorIterator<AActor> It(Lighthouse.GetWorld()); It; ++It)
    {
        for (UStaticMeshComponent* Part : TInlineComponentArray<UStaticMeshComponent*>(*It))
        {
            // Override the visual architecture only; never modify collision meshes
            // or dynamically faded reveal materials on the wreck.
            if (Part != Lighthouse.TowerExteriorSkin && Part->GetName() != TEXT("AnnexHeroDetails")) continue;
            const int32 PaintSlot = Part->GetMaterialIndex(TEXT("TowerPaint"));
            const int32 InteriorSlot = Part->GetMaterialIndex(TEXT("InteriorPlaster"));
            if (PaintSlot != INDEX_NONE) Part->SetMaterial(PaintSlot, Exterior);
            if (InteriorSlot != INDEX_NONE) Part->SetMaterial(InteriorSlot, Interior);
        }
    }
    int32 Added = 0;
    for (const FVisualPlacement& Placement : PLACEMENTS)
    {
        const FString Path = FString::Printf(TEXT("/Game/BlackBeacon/EnvironmentKitV1/Meshes/SM_%s"), Placement.Asset);
        UStaticMesh* Asset = LoadObject<UStaticMesh>(nullptr, *Path);
        if (!Asset)
        {
            UE_LOG(LogTemp, Error, TEXT("Environment kit mesh missing: %s"), *Path);
            continue;
        }
        const FBox Bounds = Asset->GetBoundingBox();
        const double Scale = Placement.WidthCm / FMath::Max(Bounds.GetSize().X, 1.0);
        const FRotator Rotation(0, Placement.Yaw, 0);
        const FVector LocalFoot(Bounds.GetCenter().X, Bounds.GetCenter().Y, Bounds.Min.Z);
        UStaticMeshComponent* Visual = NewObject<UStaticMeshComponent>(&Lighthouse,
            *FString::Printf(TEXT("EnvironmentKit_%02d_%s"), Added, Placement.Asset));
        Lighthouse.AddInstanceComponent(Visual);
        Visual->SetupAttachment(Lighthouse.GetRootComponent());
        Visual->SetStaticMesh(Asset);
        Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Visual->SetGenerateOverlapEvents(false);
        Visual->SetCanEverAffectNavigation(false);
        Visual->ComponentTags.Add(TEXT("BB_EnvironmentKitVisual"));
        Visual->RegisterComponent();
        Visual->SetWorldScale3D(FVector(Scale));
        Visual->SetWorldRotation(Rotation);
        Visual->SetWorldLocation(Placement.Foot - Rotation.RotateVector(LocalFoot * Scale));
        ++Added;
    }
    UE_LOG(LogTemp, Display, TEXT("Environment kit assembled: %d noncolliding scan instances."), Added);
}
}
