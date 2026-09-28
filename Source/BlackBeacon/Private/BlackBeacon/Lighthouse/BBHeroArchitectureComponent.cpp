#include "BlackBeacon/Lighthouse/BBHeroArchitectureComponent.h"

#include "BlackBeacon/Lighthouse/BBLighthouseController.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Materials/MaterialInterface.h"
#include "TimerManager.h"

UBBHeroArchitectureComponent::UBBHeroArchitectureComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UBBHeroArchitectureComponent::BeginPlay()
{
    Super::BeginPlay();
    // Apply after saved-map adapters have completed their existing transforms.
    GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UBBHeroArchitectureComponent::Assemble);
}

void UBBHeroArchitectureComponent::Assemble()
{
    ABBLighthouseController* Lighthouse = Cast<ABBLighthouseController>(GetOwner());
    if (!Lighthouse) return;
    const auto AddMesh = [](const TCHAR* Name, const TCHAR* Asset, USceneComponent* Parent, FVector Offset)
    {
        AActor* Owner = Parent->GetOwner();
        UStaticMeshComponent* Mesh = NewObject<UStaticMeshComponent>(Owner, Name);
        Owner->AddInstanceComponent(Mesh);
        Mesh->SetupAttachment(Parent);
        Mesh->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, Asset));
        Mesh->SetRelativeLocation(Offset);
        Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Mesh->SetCanEverAffectNavigation(false);
        Mesh->RegisterComponent();
        return Mesh;
    };
    AddMesh(TEXT("HeroStairStructure"), TEXT("/Game/BlackBeacon/Art/Lighthouse/Meshes/SM_BB_LH_StairStructure.SM_BB_LH_StairStructure"),
        Lighthouse->GetRootComponent(), FVector(0,0,-1980));
    UStaticMeshComponent* Deck = AddMesh(TEXT("HeroGalleryDeck"),
        TEXT("/Game/BlackBeacon/Art/Lighthouse/Meshes/SM_BB_LH_GalleryDeck.SM_BB_LH_GalleryDeck"),
        Lighthouse->GetRootComponent(), FVector(0,0,-1980));
    Deck->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Deck->SetCollisionResponseToAllChannels(ECR_Block);
    Deck->SetVisibility(false);
    Deck->SetCastShadow(false);
    for (USceneComponent* Part : TInlineComponentArray<USceneComponent*>(Lighthouse))
    {
        if (Part->GetName() == TEXT("BeamVisualPivot"))
        {
            UStaticMeshComponent* Rotor = AddMesh(TEXT("HeroFresnelRotor"), TEXT("/Game/BlackBeacon/Art/Lighthouse/Meshes/SM_BB_LH_FresnelRotor.SM_BB_LH_FresnelRotor"), Part, FVector::ZeroVector);
            // The carriage turns on a level roller track. Optical elevation passes
            // through its broad aperture without tipping the bearing off the pedestal.
            Rotor->SetRelativeRotation(FRotator(-Lighthouse->BeamComponent->GetCurrentPitchDegrees(),0,0));
            Lighthouse->BeamComponent->OnBeamQueryChanged.AddWeakLambda(Rotor,
                [Rotor, Lighthouse](const BlackBeacon::Logics::FBBBeamQuery& Query)
                {
                    Rotor->SetRelativeRotation(FRotator(-Lighthouse->BeamComponent->GetCurrentPitchDegrees(),0,0));
                });
            break;
        }
    }
    Lighthouse->TowerExteriorSkin->EmptyOverrideMaterials();
    // Additive exterior finish preserves the validated shell, doorway and access meshes.
    const TCHAR* ExteriorModules[] = {TEXT("Foundation"), TEXT("GalleryCorbels"), TEXT("WindowDressings")};
    for (const TCHAR* Module : ExteriorModules)
    {
        const FString Name = FString::Printf(TEXT("HeroExterior%s"), Module);
        const FString Asset = FString::Printf(TEXT("/Game/BlackBeacon/Art/Lighthouse/Meshes/SM_BB_Hero_%s.SM_BB_Hero_%s"), Module, Module);
        AddMesh(*Name, *Asset, Lighthouse->GetRootComponent(), FVector(0,0,-1980));
    }
    UMaterialInterface* ExteriorFinish = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Game/BlackBeacon/Art/Lighthouse/Materials/M_Hero_ExteriorFinish.M_Hero_ExteriorFinish"));
    const int32 PaintSlot = Lighthouse->TowerExteriorSkin->GetMaterialIndex(TEXT("TowerPaint"));
    if (ExteriorFinish && PaintSlot != INDEX_NONE)
    {
        Lighthouse->TowerExteriorSkin->SetMaterial(PaintSlot, ExteriorFinish);
    }
    // Saved component transforms can override constructor defaults during map load.
    for (UStaticMeshComponent* Mesh : TInlineComponentArray<UStaticMeshComponent*>(Lighthouse))
    {
        if (Mesh->GetName().StartsWith(TEXT("LanternGlass_")))
        {
            // Glazing now belongs to the sixteen modeled structural bays.
            Mesh->SetVisibility(false);
        }
        if (Mesh->GetName() == TEXT("BeamLensSupport") || Mesh->GetName() == TEXT("LanternFrame")
            || Mesh->GetName() == TEXT("ControlMesh"))
            Mesh->SetVisibility(false);
        if (Mesh->GetName() == TEXT("BeamLensMesh"))
        {
            Mesh->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,
                TEXT("/Game/BlackBeacon/Art/Lighthouse/Meshes/SM_BB_LH_ArcSource.SM_BB_LH_ArcSource")));
            Mesh->SetRelativeScale3D(FVector::OneVector);
        }
        if (Mesh->GetName() == TEXT("LanternFresnelBands"))
        {
            UInstancedStaticMeshComponent* Bands = Cast<UInstancedStaticMeshComponent>(Mesh);
            if (!Bands) continue;
            Bands->ClearInstances();
        }
    }
    for (TActorIterator<AActor> It(GetWorld()); It; ++It)
    {
        if (It->ActorHasTag(TEXT("BB_Generator")))
        {
            // Retain the interaction collider and the existing power-driven flywheel timer.
            for (UStaticMeshComponent* Mesh : TInlineComponentArray<UStaticMeshComponent*>(*It))
            {
                if (Mesh->GetName().StartsWith(TEXT("Generator")))
                {
                    Mesh->SetVisibility(false);
                    Mesh->SetCastShadow(false);
                }
            }
            UStaticMeshComponent* Works = AddMesh(TEXT("HeroGeneratorWorks"),
                TEXT("/Game/BlackBeacon/Art/Lighthouse/Meshes/SM_BB_GeneratorWorks.SM_BB_GeneratorWorks"),
                It->GetRootComponent(), FVector::ZeroVector);
            Works->SetAbsolute(false, false, true);
            Works->SetWorldScale3D(FVector::OneVector);
            for (USceneComponent* Part : TInlineComponentArray<USceneComponent*>(*It))
            {
                if (Part->GetName() == TEXT("GeneratorFlywheelPivot"))
                {
                    Part->SetAbsolute(false, false, true);
                    Part->SetWorldScale3D(FVector::OneVector);
                    Part->SetWorldLocation(It->GetActorLocation() + FVector(-92,0,-4));
                    AddMesh(TEXT("HeroGeneratorFlywheel"),
                        TEXT("/Game/BlackBeacon/Art/Lighthouse/Meshes/SM_BB_GeneratorFlywheel.SM_BB_GeneratorFlywheel"),
                        Part, FVector::ZeroVector);
                    break;
                }
            }
        }
        const bool bHide = It->ActorHasTag(TEXT("BB_StairStep")) || It->ActorHasTag(TEXT("BB_StairGuard"))
            || It->ActorHasTag(TEXT("BB_StairCore")) || It->ActorHasTag(TEXT("BB_StairEntryLanding"))
            || It->ActorHasTag(TEXT("BB_LanternFloor"));
        for (UStaticMeshComponent* Mesh : TInlineComponentArray<UStaticMeshComponent*>(*It))
        {
            const FString Name = Mesh->GetName();
            if (bHide || (Name.StartsWith(TEXT("Annex")) && Name != TEXT("AnnexHeroDetails")))
            {
                Mesh->SetVisibility(false);
                Mesh->SetCastShadow(false);
            }
            if (Name == TEXT("CoastRockField"))
            {
                UInstancedStaticMeshComponent* Original = Cast<UInstancedStaticMeshComponent>(Mesh);
                if (!Original) continue;
                UInstancedStaticMeshComponent* Rocks = NewObject<UInstancedStaticMeshComponent>(Lighthouse, TEXT("HeroCoastalRocks"));
                Lighthouse->AddInstanceComponent(Rocks);
                Rocks->SetupAttachment(Lighthouse->GetRootComponent());
                Rocks->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Game/BlackBeacon/Art/Lighthouse/Meshes/SM_BB_CoastalRock.SM_BB_CoastalRock")));
                Rocks->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                Rocks->SetCanEverAffectNavigation(false);
                Rocks->RegisterComponent();
                for (int32 I=0; I<Original->GetInstanceCount(); ++I)
                {
                    FTransform Transform;
                    Original->GetInstanceTransform(I,Transform,true);
                    Rocks->AddInstance(Transform,true);
                }
                Original->SetVisibility(false);
                Original->SetCastShadow(false);
            }
        }
        for (UPointLightComponent* Light : TInlineComponentArray<UPointLightComponent*>(*It))
        {
            if (Light->GetName() == TEXT("GeneratorShedLight"))
            {
                Light->SetIntensity(AnnexPracticalLumens);
                Light->SetAttenuationRadius(PracticalRadiusCm);
                Light->AddWorldOffset(AnnexPracticalOffset);
            }
            if (Light->GetName() == TEXT("StairFillLight"))
            {
                Light->SetIntensity(PracticalLumens);
                Light->SetAttenuationRadius(PracticalRadiusCm);
                Light->SetWorldLocation(FVector(-125,-120,It->GetActorLocation().Z+65));
                Light->SetLightColor(FLinearColor(1.0f,0.56f,0.25f));
            }
        }
    }
    const FVector Positions[] = {{343,-85,262},{-196,-523,235},{-125,-120,1690}};
    for (int32 I=0; I<UE_ARRAY_COUNT(Positions); ++I)
    {
        UPointLightComponent* Light = NewObject<UPointLightComponent>(Lighthouse, *FString::Printf(TEXT("HeroPractical_%d"),I));
        Lighthouse->AddInstanceComponent(Light);
        Light->SetupAttachment(Lighthouse->GetRootComponent());
        Light->SetMobility(EComponentMobility::Movable);
        Light->IntensityUnits = ELightUnits::Lumens;
        Light->SetIntensity(PracticalLumens);
        Light->SetAttenuationRadius(PracticalRadiusCm);
        Light->SetLightColor(FLinearColor(1.0f,0.56f,0.25f));
        Light->SetCastShadows(true);
        Light->RegisterComponent();
        Light->SetWorldLocation(Positions[I]);
    }
}
