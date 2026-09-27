#include "BlackBeacon/Lighthouse/BBLighthouseCollisionComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "TimerManager.h"

void UBBLighthouseCollisionComponent::BeginPlay()
{
    Super::BeginPlay();
    GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UBBLighthouseCollisionComponent::Assemble);
}

void UBBLighthouseCollisionComponent::Assemble()
{
    AActor* Owner = GetOwner();
    auto* Shell = NewObject<UInstancedStaticMeshComponent>(Owner, TEXT("TowerAndLanternCollision"));
    Owner->AddInstanceComponent(Shell);
    Shell->SetupAttachment(Owner->GetRootComponent());
    Shell->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")));
    Shell->SetCollisionProfileName(TEXT("BlockAll"));
    Shell->SetVisibility(false);
    Shell->SetHiddenInGame(true);
    Shell->SetCastShadow(false);
    Shell->ComponentTags.Add(TEXT("BB_NoInteraction"));
    Shell->RegisterComponent();
    const FVector Ground(0, 0, -1980);
    const auto Box = [&](float Angle, float Radius, float Bottom, float Top, float Depth, float Width, float Pitch = 0.0f)
    {
        const FVector Position = Ground + FVector(FMath::Cos(FMath::DegreesToRadians(Angle))*Radius,
            FMath::Sin(FMath::DegreesToRadians(Angle))*Radius, (Bottom+Top)*0.5f);
        Shell->AddInstance(FTransform(FRotator(Pitch, Angle, 0), Position,
            FVector(Depth, Width, (Top-Bottom)/FMath::Cos(FMath::DegreesToRadians(Pitch)))/100.0f));
    };
    // 26 cm masonry, tapered inward by 92 cm over the tower's height.
    for (int32 Segment=0; Segment<64; ++Segment)
    {
        const float Angle=(Segment+0.5f)*360.0f/64.0f;
        for (int32 Band=0; Band<2; ++Band)
        {
            if (Band==0 && (Segment<2 || Segment>=62)) continue;
            const float Bottom=Band==0 ? 0.0f : 242.0f;
            const float Top=Band==0 ? 242.0f : 1560.0f;
            const float Radius=301.0f-92.0f*(Bottom+Top)*0.5f/1560.0f;
            const bool bDoorJamb = Band==0 && (Segment==2 || Segment==61);
            const float JambAngle = bDoorJamb ? (Segment==2 ? 0.7f : -0.7f) : 0.0f;
            // Match the visible 0.22-radian opening instead of rounding it inward.
            Box(Angle+JambAngle, Radius, Bottom, Top, 26,
                2*(314-92*Bottom/1560)*FMath::Sin(PI/64)+2-(bDoorJamb ? 10.0f : 0.0f),
                FMath::RadiansToDegrees(FMath::Atan(92.0f/1560.0f)));
        }
    }
    for (int32 Segment=0; Segment<16; ++Segment)
    {
        // The +X service door is two bays wide; its lintel clears a standing capsule.
        Box((Segment+0.5f)*22.5f, 244, (Segment==0 || Segment==15) ? 1800 : 1570,
            2113, 14, 99);
        Box((Segment+0.5f)*22.5f, 385, 1570, 1680, 8, 154);
    }
    TArray<UStaticMeshComponent*> Meshes;
    Owner->GetComponents(Meshes);
    for (UStaticMeshComponent* Mesh : Meshes)
    {
        if (Mesh->GetFName()==TEXT("ControlMesh"))
        {
            Mesh->SetRelativeLocation(FVector(-175,-85,-350));
            Mesh->SetRelativeScale3D(FVector(0.56f,0.98f,1.0f));
            Mesh->SetCollisionProfileName(TEXT("BlockAll"));
        }
    }
    // Old blockout guards protruded into the capsule lane, especially on flight three.
    AActor* EntryGuard = nullptr;
    for (TActorIterator<AActor> It(GetWorld()); It; ++It)
    {
        if (It->ActorHasTag(TEXT("BB_StairGuard")))
        {
            FVector Radial=It->GetActorLocation(); Radial.Z=0;
            It->AddActorWorldOffset(Radial.GetSafeNormal()*8.0f);
            if (!EntryGuard || It->GetActorLocation().Z < EntryGuard->GetActorLocation().Z)
            {
                EntryGuard = *It;
            }
        }
    }
    if (UStaticMeshComponent* GuardMesh = EntryGuard ? EntryGuard->FindComponentByClass<UStaticMeshComponent>() : nullptr)
    {
        // The rail begins at its first post. The blockout guard extended half a
        // tread beyond that post and caught entering capsules on an invisible corner.
        // Retain the trailing half toward the next post; preserve every upper guard.
        FVector Scale = GuardMesh->GetComponentScale();
        const FVector Offset = GuardMesh->GetRightVector()
            * GuardMesh->GetStaticMesh()->GetBounds().BoxExtent.Y * Scale.Y * 0.5f;
        Scale.Y *= 0.5f;
        GuardMesh->SetWorldScale3D(Scale);
        GuardMesh->AddWorldOffset(-Offset);
    }
}
