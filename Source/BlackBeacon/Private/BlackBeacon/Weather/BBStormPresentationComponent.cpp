#include "BlackBeacon/Weather/BBStormPresentationComponent.h"
#include "BlackBeacon/Weather/BBWeatherController.h"
#include "BlackBeacon/Weather/BBStormAudioComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "Materials/MaterialInstanceDynamic.h"

namespace
{
    const FVector IMPACTS[] = {
        {-4800,-1450,-210}, {-3400,-1250,-210}, {-1650,-1050,-210},
        {400,-2080,-210}, {2380,-900,-210}, {2370,1600,-210}, {-5500,3450,-210}
    };
    constexpr int32 PUFFS_PER_IMPACT = 22;
}

UBBStormPresentationComponent::UBBStormPresentationComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.05f;
}

void UBBStormPresentationComponent::BeginPlay()
{
    Super::BeginPlay();
    weather = Cast<ABBWeatherController>(GetOwner());
    if (!weather) { SetComponentTickEnabled(false); return; }
    // Serialized legacy sky component is retained for reversible migration, hidden at runtime.
    weather->SkyCloudDome->SetVisibility(false);
    weather->SkyCloudDome->SetHiddenInGame(true);
    weather->StormClouds->SetLayerBottomAltitude(CloudBottomKm);
    weather->StormClouds->SetLayerHeight(CloudHeightKm);
    weather->StormClouds->SetTracingMaxDistance(14.0f);
    weather->StormClouds->SetViewSampleCountScale(CloudSampleScale);
    weather->StormClouds->SetReflectionViewSampleCountScale(0.25f);
    weather->StormClouds->SetShadowViewSampleCountScale(0.4f);
    weather->StormClouds->SetSkyLightCloudBottomOcclusion(0.45f);
    UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Game/BlackBeacon/Storm/Materials/MI_StormCloudNative.MI_StormCloudNative"));
    if (Base)
    {
        cloudsMaterial = UMaterialInstanceDynamic::Create(Base, this);
        weather->StormClouds->SetMaterial(cloudsMaterial);
    }
    weather->MoonLight->SetCastShadows(true);
    weather->SkyLight->SetLowerHemisphereColor(GroundBounceColor);
    weather->SkyAtmosphere->SetRayleighScattering(RayleighColor);
    weather->MoonLight->bCastCloudShadows = true;
    weather->MoonLight->CloudShadowStrength = 0.7f;
    weather->MoonLight->CloudShadowMapResolutionScale = 0.5f;
    weather->MoonLight->CloudShadowExtent = 8.0f;
    weather->FogComponent->SetFogHeightFalloff(0.45f);

    lightningLight = NewObject<UDirectionalLightComponent>(GetOwner(), TEXT("StormLightning"));
    GetOwner()->AddInstanceComponent(lightningLight);
    lightningLight->SetupAttachment(GetOwner()->GetRootComponent());
    lightningLight->SetMobility(EComponentMobility::Movable);
    lightningLight->SetLightColor(FLinearColor(0.65f,0.76f,1.0f));
    lightningLight->SetCastShadows(false);
    lightningLight->bCastCloudShadows = false;
    lightningLight->SetAtmosphereSunLight(true);
    lightningLight->SetAtmosphereSunLightIndex(1);
    lightningLight->SetIntensity(0.0f);
    lightningLight->RegisterComponent();
    lightningLight->SetWorldRotation(FRotator(-25,135,0));

    stormAudio = NewObject<UBBStormAudioComponent>(GetOwner(), TEXT("StormAudio"));
    GetOwner()->AddInstanceComponent(stormAudio);
    stormAudio->RegisterComponent();

    const auto Field = [this](const TCHAR* Name, const TCHAR* Material, int32 Count)
    {
        UInstancedStaticMeshComponent* Mesh = NewObject<UInstancedStaticMeshComponent>(GetOwner(), Name);
        GetOwner()->AddInstanceComponent(Mesh);
        Mesh->SetupAttachment(GetOwner()->GetRootComponent());
        Mesh->SetMobility(EComponentMobility::Movable);
        Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Mesh->SetCanEverAffectNavigation(false);
        Mesh->SetCastShadow(false);
        Mesh->NumCustomDataFloats = 1;
        Mesh->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Plane.Plane")));
        Mesh->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,Material));
        Mesh->RegisterComponent();
        Mesh->SetWorldTransform(FTransform::Identity);
        for (int32 I=0; I<Count; ++I) Mesh->AddInstance(FTransform(FRotator::ZeroRotator,FVector::ZeroVector,FVector::ZeroVector));
        return Mesh;
    };
    spray = Field(TEXT("CoastalImpactSpray"), TEXT("/Game/BlackBeacon/Storm/Materials/M_SeaSpray.M_SeaSpray"), UE_ARRAY_COUNT(IMPACTS)*PUFFS_PER_IMPACT);
    mist = Field(TEXT("CoastalDriftingMist"), TEXT("/Game/BlackBeacon/Storm/Materials/M_CoastalMist.M_CoastalMist"), UE_ARRAY_COUNT(IMPACTS));
    splashes = Field(TEXT("StormSurfaceSplashes"), TEXT("/Game/BlackBeacon/Storm/Materials/M_RainSplash.M_RainSplash"), 96);
    sprayTransforms.SetNum(spray->GetInstanceCount());
}

void UBBStormPresentationComponent::InitializeSurfaces()
{
    UStaticMesh* OceanMesh = LoadObject<UStaticMesh>(nullptr,TEXT("/Game/BlackBeacon/Storm/Meshes/SM_StormOcean.SM_StormOcean"));
    UStaticMesh* CoastMesh = LoadObject<UStaticMesh>(nullptr,TEXT("/Game/BlackBeacon/Storm/Meshes/SM_StormCoast.SM_StormCoast"));
    UMaterialInterface* Water = LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/BlackBeacon/Storm/Materials/M_StormOcean.M_StormOcean"));
    for (TActorIterator<AActor> It(GetWorld()); It; ++It)
    {
        for (UStaticMeshComponent* Mesh : TInlineComponentArray<UStaticMeshComponent*>(*It))
        {
            if (Mesh->GetName()==TEXT("CoastalOcean") && OceanMesh && Water)
            {
                Mesh->SetStaticMesh(OceanMesh);
                Mesh->SetWorldTransform(FTransform(FRotator::ZeroRotator,FVector(0,0,-240),FVector::OneVector));
                Mesh->SetBoundsScale(1.2f);
                oceanMaterial = UMaterialInstanceDynamic::Create(Water,this);
                Mesh->SetMaterial(0,oceanMaterial);
            }
            if (It->ActorHasTag(TEXT("BB_Ocean"))) Mesh->SetVisibility(false);
            if (It->ActorHasTag(TEXT("BB_Ground")) && CoastMesh)
            {
                // Replace only the terrain surface: the safe shore-to-generator route stays at Z=0.
                Mesh->SetStaticMesh(CoastMesh);
                Mesh->SetWorldScale3D(FVector::OneVector);
                Mesh->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,
                    TEXT("/Game/BlackBeacon/Storm/Materials/MI_StormGround.MI_StormGround")));
            }
        }
    }
    FRandomStream Random(7123);
    for (int32 I=0; I<splashes->GetInstanceCount(); ++I)
    {
        const FVector Start(Random.FRandRange(-5000,2000),Random.FRandRange(-1400,1400),6000);
        FHitResult Hit;
        if (GetWorld()->LineTraceSingleByChannel(Hit,Start,Start-FVector(0,0,6800),ECC_WorldStatic)
            && Hit.ImpactPoint.Z<180 && Hit.ImpactNormal.Z>0.6f)
        {
            splashes->UpdateInstanceTransform(I,FTransform(FRotator::ZeroRotator,Hit.ImpactPoint+FVector(0,0,3),FVector(0.55f)),true);
            splashes->SetCustomDataValue(I,0,Random.FRand(),false);
        }
    }
    splashes->MarkRenderStateDirty();
    surfacesReady = true;
}

bool UBBStormPresentationComponent::IsReady() const
{
    return surfacesReady && oceanMaterial && cloudsMaterial && spray && stormAudio && stormAudio->HasAmbientLayers();
}

void UBBStormPresentationComponent::TriggerLightning(float DistanceMetres)
{
    timing.Trigger(DistanceMetres);
    thunderPosition = FVector(-0.8f,0.6f,0.15f).GetSafeNormal()*DistanceMetres*100.0f;
}

float UBBStormPresentationComponent::GetLightningIntensity() const
{
    return static_cast<float>(timing.Flash());
}

void UBBStormPresentationComponent::TickComponent(float DeltaTime,ELevelTick TickType,FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime,TickType,ThisTickFunction);
    if (!weather) return;
    elapsed += DeltaTime;
    if (!surfacesReady)
    {
        initializationDelay += DeltaTime;
        if (initializationDelay<0.25f) return;
        InitializeSurfaces();
    }
    FVector Listener = FVector(-4500,-600,240);
    if (APlayerController* PC=GetWorld()->GetFirstPlayerController())
    {
        if (PC->PlayerCameraManager) Listener=PC->PlayerCameraManager->GetCameraLocation();
    }
    const FVector Wind = weather->GetWindVelocity();
    const float Strength = weather->GetWindStrength();
    timing.Tick(DeltaTime,weather->GetTargetPhase()==EBBWeatherPhase::Storm);
    if (timing.FlashStarted) thunderPosition = Listener+FVector(-0.8f,0.6f,0.15f).GetSafeNormal()*timing.DistanceMetres*100.0;
    if (timing.ThunderDue) stormAudio->PlayThunder(thunderPosition,0.87f+0.16f*FMath::Frac(elapsed*0.173f));
    lightningLight->SetIntensity(LightningLux*GetLightningIntensity());
    if (cloudsMaterial)
    {
        const FVector Direction=Wind.GetSafeNormal();
        cloudsMaterial->SetVectorParameterValue(TEXT("Layout_WindControls"),FLinearColor(Direction.X,Direction.Y,0,CloudWindSpeed));
        cloudsMaterial->SetVectorParameterValue(TEXT("Storm_LightningAnim"),FLinearColor(GetLightningIntensity(),0,0,0));
        cloudsMaterial->SetScalarParameterValue(TEXT("Cloud_GlobalCoverage"),FMath::Lerp(ClearCloudCoverage,StormCloudCoverage,Strength));
    }
    if (oceanMaterial)
    {
        const FVector Direction=Wind.GetSafeNormal();
        oceanMaterial->SetVectorParameterValue(TEXT("WindDirection"),FLinearColor(Direction.X,Direction.Y,0));
        oceanMaterial->SetScalarParameterValue(TEXT("StormStrength"),0.35f+0.65f*Strength);
    }
    stormAudio->UpdateMix(DeltaTime,Strength,weather->GetRainIntensity(),weather->IsListenerSheltered(),Listener);
    splashes->SetVisibility(weather->GetRainIntensity()>0.05f);
    UpdateSpray(DeltaTime,Listener);
}

void UBBStormPresentationComponent::UpdateSpray(float DeltaTime,const FVector& Listener)
{
    const FVector Wind=weather->GetWindVelocity();
    const float Strength=weather->GetWindStrength();
    for (int32 Site=0; Site<UE_ARRAY_COUNT(IMPACTS); ++Site)
    {
        const float Period=7.1f+Site*0.69f;
        const float Phase=FMath::Fmod(elapsed+Site*2.37f,Period);
        for (int32 J=0; J<PUFFS_PER_IMPACT; ++J)
        {
            const int32 I=Site*PUFFS_PER_IMPACT+J;
            const float Age=Phase-J*0.045f;
            const float Life=2.6f+J*0.025f;
            const bool Active=Age>0 && Age<Life;
            const float Angle=J*2.399963f;
            const float A=FMath::Max(Age,0.0f);
            const FVector Jet(FMath::Cos(Angle)*100,FMath::Sin(Angle)*110,400+J*8);
            FVector Position=IMPACTS[Site]+Jet*A+Wind*A*0.18f-FVector(0,0,120*A*A);
            const float Fade=Active ? FMath::Sin(PI*Age/Life)*Strength : 0;
            const float Size=(0.35f+A*1.5f)*(1.0f+(J%4)*0.35f);
            const FQuat Facing=FQuat::FindBetweenNormals(FVector::UpVector,(Listener-Position).GetSafeNormal());
            sprayTransforms[I]=FTransform(Facing,Position,FVector(Size,Size*1.35f,1));
            spray->SetCustomDataValue(I,0,Fade,false);
        }
        const FVector Position=IMPACTS[Site]+FVector(FMath::Sin(elapsed*.05f+Site)*400,0,160)+Wind.GetSafeNormal()*Phase*75;
        const FQuat Facing=FQuat::FindBetweenNormals(FVector::UpVector,(Listener-Position).GetSafeNormal());
        mist->UpdateInstanceTransform(Site,FTransform(Facing,Position,FVector(22,6,1)),true,false,true);
        mist->SetCustomDataValue(Site,0,0.5f+0.2f*FMath::Sin(elapsed*.17f+Site),false);
    }
    spray->BatchUpdateInstancesTransforms(0,sprayTransforms,true,true,true);
    mist->MarkRenderStateDirty();
    (void)DeltaTime;
}
