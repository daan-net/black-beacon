#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BBBeamControlComponent.generated.h"

class ABBLighthouseController;
class ACameraActor;

// A temporary optical sight: input remains player-driven; leaving restores the pawn view.
UCLASS(config = Game)
class UBBBeamControlComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UBBBeamControlComponent();
    void Acquire(ABBLighthouseController* Lighthouse);
    void Release();
    void Aim(const FVector2D& DeltaDegrees);
    bool IsActive() const { return lighthouse.IsValid(); }
protected:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;
private:
    void UpdateView();
    TWeakObjectPtr<ABBLighthouseController> lighthouse;
    TWeakObjectPtr<AActor> previousView;
    FRotator previousRotation;
    float targetYaw = 0.0f;
    float previousScattering = 0.0f;
    UPROPERTY(config)
    float SearchScatteringScale = 0.15f;
    UPROPERTY(Transient)
    TObjectPtr<ACameraActor> sightCamera;
    UPROPERTY(config)
    float ViewBlendSeconds = 0.35f;
    UPROPERTY(config)
    float SightOffsetCm = 290.0f;
};
