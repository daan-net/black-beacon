#include "BlackBeacon/Weather/BBStormAudioComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "Sound/SoundAttenuation.h"
#include "GameFramework/Actor.h"

UBBStormAudioComponent::UBBStormAudioComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

UAudioComponent* UBBStormAudioComponent::MakeLayer(const TCHAR* Name, const TCHAR* AssetName,
    bool bSpatial, const FVector& Position)
{
    UAudioComponent* Audio = NewObject<UAudioComponent>(GetOwner(), Name);
    GetOwner()->AddInstanceComponent(Audio);
    Audio->SetupAttachment(GetOwner()->GetRootComponent());
    Audio->bAutoActivate = false;
    Audio->bAutoDestroy = false;
    Audio->bAllowSpatialization = bSpatial;
    Audio->SetSound(LoadObject<USoundWave>(nullptr,
        *FString::Printf(TEXT("/Game/BlackBeacon/Storm/Audio/%s.%s"), AssetName, AssetName)));
    if (bSpatial)
    {
        FSoundAttenuationSettings Attenuation;
        Attenuation.bAttenuate = true;
        Attenuation.bSpatialize = true;
        Attenuation.AttenuationShapeExtents = FVector(800.0f);
        Attenuation.FalloffDistance = 18000.0f;
        Audio->SetAttenuationOverrides(Attenuation);
    }
    Audio->RegisterComponent();
    Audio->SetWorldLocation(Position);
    Audio->SetVolumeMultiplier(0.0f);
    return Audio;
}

void UBBStormAudioComponent::BeginPlay()
{
    Super::BeginPlay();
    windAudio = MakeLayer(TEXT("StormWindAudio"), TEXT("SW_StormWind"), false, FVector::ZeroVector);
    rainAudio = MakeLayer(TEXT("StormRainAudio"), TEXT("SW_StormRain"), false, FVector::ZeroVector);
    surfAudio = MakeLayer(TEXT("StormSurfAudio"), TEXT("SW_StormSurf"), true, FVector(-3300,-1250,-200));
    thunderAudio = MakeLayer(TEXT("StormThunderAudio"), TEXT("SW_StormThunder"), true, FVector::ZeroVector);
    FSoundAttenuationSettings ThunderAttenuation;
    ThunderAttenuation.AttenuationShapeExtents = FVector(200000.0f);
    ThunderAttenuation.FalloffDistance = 100000.0f;
    thunderAudio->SetAttenuationOverrides(ThunderAttenuation);
    for (UAudioComponent* Audio : {windAudio.Get(), rainAudio.Get(), surfAudio.Get()})
    {
        Audio->Play();
    }
}

bool UBBStormAudioComponent::HasAmbientLayers() const
{
    return windAudio && windAudio->Sound && rainAudio && rainAudio->Sound
        && surfAudio && surfAudio->Sound && thunderAudio && thunderAudio->Sound;
}

void UBBStormAudioComponent::UpdateMix(float DeltaSeconds, float Wind, float Rain,
    bool bSheltered, const FVector& Listener)
{
    if (!HasAmbientLayers()) return;
    shelterMix = FMath::FInterpTo(shelterMix, bSheltered ? 1.0f : 0.0f, DeltaSeconds, 1.8f);
    const float Exterior = 1.0f - shelterMix;
    windAudio->SetVolumeMultiplier((0.32f + 1.1f * Wind) * FMath::Lerp(0.22f, 1.0f, Exterior));
    rainAudio->SetVolumeMultiplier(Rain * FMath::Lerp(0.09f, 0.62f, Exterior));
    surfAudio->SetVolumeMultiplier((0.45f + Wind) * FMath::Lerp(0.25f, 1.0f, Exterior));
    for (UAudioComponent* Audio : {windAudio.Get(), rainAudio.Get(), surfAudio.Get(), thunderAudio.Get()})
    {
        Audio->SetLowPassFilterEnabled(true);
        Audio->SetLowPassFilterFrequency(FMath::Lerp(15000.0f, 900.0f, shelterMix));
    }
    // The extended shoreline remains audible without a point source following the head.
    (void)Listener;
}

void UBBStormAudioComponent::PlayThunder(const FVector& Location, float Pitch)
{
    if (!thunderAudio || !thunderAudio->Sound) return;
    thunderAudio->SetWorldLocation(Location);
    thunderAudio->SetVolumeMultiplier(FMath::Lerp(1.35f, 0.65f, shelterMix));
    thunderAudio->SetPitchMultiplier(Pitch);
    thunderAudio->Play();
    ++thunderCount;
}
