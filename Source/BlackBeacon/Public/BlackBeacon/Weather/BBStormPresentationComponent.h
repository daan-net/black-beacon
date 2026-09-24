#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BlackBeacon/Logics/BBStormTiming.h"
#include "BBStormPresentationComponent.generated.h"

class ABBWeatherController;
class UInstancedStaticMeshComponent;
class UMaterialInstanceDynamic;
class UDirectionalLightComponent;
class UBBStormAudioComponent;

UCLASS(config=Game)
class UBBStormPresentationComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UBBStormPresentationComponent();
    void TriggerLightning(float DistanceMetres);
    float GetLightningIntensity() const;
    UBBStormAudioComponent* GetStormAudio() const { return stormAudio; }
    bool IsReady() const;
    UPROPERTY(config, EditAnywhere, Category="Storm") float LightningLux = 14.0f;
    UPROPERTY(config, EditAnywhere, Category="Storm") float CloudBottomKm = 0.45f;
    UPROPERTY(config, EditAnywhere, Category="Storm") float CloudHeightKm = 1.3f;
    UPROPERTY(config, EditAnywhere, Category="Storm") float CloudSampleScale = 2.0f;
    UPROPERTY(config, EditAnywhere, Category="Storm") float CloudWindSpeed = 0.35f;
    UPROPERTY(config, EditAnywhere, Category="Storm") float ClearCloudCoverage = -0.3f;
    UPROPERTY(config, EditAnywhere, Category="Storm") float StormCloudCoverage = 0.15f;
    UPROPERTY(config, EditAnywhere, Category="Storm") FLinearColor GroundBounceColor = FLinearColor(0.06f, 0.075f, 0.10f);
    UPROPERTY(config, EditAnywhere, Category="Storm") FLinearColor RayleighColor = FLinearColor(0.012f, 0.015f, 0.022f);
protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
private:
    void InitializeSurfaces();
    void UpdateSpray(float DeltaTime, const FVector& Listener);
    UPROPERTY(Transient) TObjectPtr<ABBWeatherController> weather;
    UPROPERTY(Transient) TObjectPtr<UInstancedStaticMeshComponent> spray;
    UPROPERTY(Transient) TObjectPtr<UInstancedStaticMeshComponent> mist;
    UPROPERTY(Transient) TObjectPtr<UInstancedStaticMeshComponent> splashes;
    UPROPERTY(Transient) TObjectPtr<UMaterialInstanceDynamic> cloudsMaterial;
    UPROPERTY(Transient) TObjectPtr<UMaterialInstanceDynamic> oceanMaterial;
    UPROPERTY(Transient) TObjectPtr<UBBStormAudioComponent> stormAudio;
    TArray<FTransform> sprayTransforms;
    FVector thunderPosition = FVector::ZeroVector;
    BlackBeacon::Logics::FBBStormTiming timing;
    float elapsed = 0.0f;
    float initializationDelay = 0.0f;
    bool surfacesReady = false;
};
