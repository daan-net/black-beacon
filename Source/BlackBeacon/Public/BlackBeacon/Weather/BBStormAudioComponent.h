#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BBStormAudioComponent.generated.h"

class UAudioComponent;

UCLASS()
class UBBStormAudioComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UBBStormAudioComponent();
    void UpdateMix(float DeltaSeconds, float Wind, float Rain, bool bSheltered, const FVector& Listener);
    void PlayThunder(const FVector& Location, float Pitch);
    int32 GetThunderCount() const { return thunderCount; }
    bool HasAmbientLayers() const;
    float GetShelterMix() const { return shelterMix; }
protected:
    virtual void BeginPlay() override;
private:
    UAudioComponent* MakeLayer(const TCHAR* Name, const TCHAR* AssetName, bool bSpatial, const FVector& Position);
    UPROPERTY(Transient) TObjectPtr<UAudioComponent> windAudio;
    UPROPERTY(Transient) TObjectPtr<UAudioComponent> rainAudio;
    UPROPERTY(Transient) TObjectPtr<UAudioComponent> surfAudio;
    UPROPERTY(Transient) TObjectPtr<UAudioComponent> thunderAudio;
    float shelterMix = 0.0f;
    int32 thunderCount = 0;
};
