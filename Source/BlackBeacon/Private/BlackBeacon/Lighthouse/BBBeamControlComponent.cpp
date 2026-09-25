#include "BlackBeacon/Lighthouse/BBBeamControlComponent.h"
#include "BlackBeacon/Lighthouse/BBLighthouseController.h"
#include "BlackBeacon/Lighthouse/BBLighthouseBeamComponent.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SpotLightComponent.h"

UBBBeamControlComponent::UBBBeamControlComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void UBBBeamControlComponent::Acquire(ABBLighthouseController* Lighthouse)
{
    APlayerController* PC = Cast<APlayerController>(GetOwner());
    if (!PC || !Lighthouse || !Lighthouse->IsPowered() || !Lighthouse->IsBeamInManualMode()) return;
    if (IsActive()) Release();
    lighthouse = Lighthouse;
    previousView = PC->GetViewTarget();
    previousRotation = PC->GetControlRotation();
    targetYaw = Lighthouse->BeamComponent->GetCurrentYawDegrees();
    if (!sightCamera)
    {
        sightCamera = GetWorld()->SpawnActor<ACameraActor>();
        sightCamera->GetCameraComponent()->SetFieldOfView(65.0f);
        auto& Settings = sightCamera->GetCameraComponent()->PostProcessSettings;
        Settings.bOverride_BloomIntensity = true;
        Settings.BloomIntensity = 0.15f;
        Settings.bOverride_LensFlareIntensity = true;
        Settings.LensFlareIntensity = 0.0f;
    }
    if (ACharacter* Pawn = Cast<ACharacter>(PC->GetPawn()))
    {
        Pawn->GetCharacterMovement()->StopMovementImmediately();
    }
    // The shaft is an exterior scattering proxy. Seen end-on from inside it, its
    // accumulated translucency masks the target. Keep the actual volumetric light
    // and surface illumination; exclude only this proxy from the operator view.
    for (UStaticMeshComponent* Mesh : TInlineComponentArray<UStaticMeshComponent*>(Lighthouse))
    {
        if (Mesh->GetFName()==TEXT("BeamVisualMesh")) PC->HiddenPrimitiveComponents.AddUnique(Mesh);
    }
    if (USpotLightComponent* Light = Lighthouse->FindComponentByClass<USpotLightComponent>())
    {
        previousScattering = Light->VolumetricScatteringIntensity;
        Light->SetVolumetricScatteringIntensity(previousScattering * SearchScatteringScale);
    }
    PC->SetIgnoreMoveInput(true);
    UpdateView();
    PC->SetViewTargetWithBlend(sightCamera, ViewBlendSeconds);
    SetComponentTickEnabled(true);
}

void UBBBeamControlComponent::Release()
{
    if (!IsActive()) return;
    if (lighthouse->IsBeamInManualMode()) lighthouse->BeamComponent->SetRotationMode(EBBBeamRotationMode::Auto);
    if (APlayerController* PC = Cast<APlayerController>(GetOwner()))
    {
        PC->HiddenPrimitiveComponents.RemoveAll([this](const TWeakObjectPtr<UPrimitiveComponent>& Mesh)
        {
            return Mesh.IsValid() && Mesh->GetOwner()==lighthouse.Get() && Mesh->GetFName()==TEXT("BeamVisualMesh");
        });
        PC->SetIgnoreMoveInput(false);
        PC->SetControlRotation(previousRotation);
        PC->SetViewTargetWithBlend(previousView.IsValid() ? previousView.Get() : PC->GetPawn(), ViewBlendSeconds);
    }
    if (USpotLightComponent* Light = lighthouse->FindComponentByClass<USpotLightComponent>())
    {
        Light->SetVolumetricScatteringIntensity(previousScattering);
    }
    lighthouse.Reset();
    SetComponentTickEnabled(false);
}

void UBBBeamControlComponent::Aim(const FVector2D& DeltaDegrees)
{
    if (!IsActive()) return;
    UBBLighthouseBeamComponent* Beam = lighthouse->BeamComponent;
    targetYaw = FRotator::NormalizeAxis(targetYaw + DeltaDegrees.X);
    Beam->SetManualYawTarget(targetYaw);
    Beam->SetManualPitchDegrees(Beam->GetCurrentPitchDegrees() + DeltaDegrees.Y);
}

void UBBBeamControlComponent::UpdateView()
{
    const auto Query = lighthouse->BeamComponent->GetBeamQuery();
    const FVector Direction(Query.Direction.X, Query.Direction.Y, Query.Direction.Z);
    const FVector Origin(Query.Origin.X, Query.Origin.Y, Query.Origin.Z);
    sightCamera->SetActorLocationAndRotation(Origin + Direction * SightOffsetCm, Direction.Rotation());
}

void UBBBeamControlComponent::TickComponent(float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (!IsActive() || !lighthouse->IsPowered() || !lighthouse->IsBeamInManualMode())
    {
        Release();
        return;
    }
    UpdateView();
}
