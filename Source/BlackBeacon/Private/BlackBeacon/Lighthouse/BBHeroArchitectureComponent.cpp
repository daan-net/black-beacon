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
    const auto AddMesh = [Lighthouse](const TCHAR* Name, const TCHAR* Asset, USceneComponent* Parent, FVector Offset)
    {
        UStaticMeshComponent* Mesh = NewObject<UStaticMeshComponent>(Lighthouse, Name);
        Lighthouse->AddInstanceComponent(Mesh);
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
            AddMesh(TEXT("HeroFresnelRotor"), TEXT("/Game/BlackBeacon/Art/Lighthouse/Meshes/SM_BB_LH_FresnelRotor.SM_BB_LH_FresnelRotor"), Part, FVector::ZeroVector);
            break;
        }
    }
    Lighthouse->TowerExteriorSkin->EmptyOverrideMaterials();
    // Saved component transforms can override constructor defaults during map load.
    for (UStaticMeshComponent* Mesh : TInlineComponentArray<UStaticMeshComponent*>(Lighthouse))
    {
        if (Mesh->GetName().StartsWith(TEXT("LanternGlass_")))
        {
            const int32 Index = FCString::Atoi(*Mesh->GetName().RightChop(13));
            const float Angle = FMath::DegreesToRadians(Index * 45.0f + 22.5f);
            Mesh->SetRelativeTransform(FTransform(FRotator(0,Index*45.0f+112.5f,0),
                FVector(FMath::Cos(Angle)*243,FMath::Sin(Angle)*243,-64),FVector(1.98f,0.025f,3.60f)));
        }
        if (Mesh->GetName() == TEXT("BeamLensSupport") || Mesh->GetName() == TEXT("LanternFrame"))
            Mesh->SetVisibility(false);
        if (Mesh->GetName() == TEXT("LanternFresnelBands"))
        {
            UInstancedStaticMeshComponent* Bands = Cast<UInstancedStaticMeshComponent>(Mesh);
            if (!Bands) continue;
            Bands->ClearInstances();
            for (int32 I=0; I<8; ++I)
            {
                const float Angle = FMath::DegreesToRadians(I*45.0f+22.5f);
                for (float Z : {-64.0f,-32.0f,0.0f,32.0f,64.0f})
                    Bands->AddInstance(FTransform(FRotator(0,I*45.0f+112.5f,0),
                        FVector(FMath::Cos(Angle)*70,FMath::Sin(Angle)*70,Z),FVector(.48f,.025f,.025f)));
            }
        }
    }
    for (TActorIterator<AActor> It(GetWorld()); It; ++It)
    {
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
