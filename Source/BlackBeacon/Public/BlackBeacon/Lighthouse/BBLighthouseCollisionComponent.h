#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BBLighthouseCollisionComponent.generated.h"

// Simple collision follows the authored shell, independently of the hero render mesh.
UCLASS()
class UBBLighthouseCollisionComponent : public UActorComponent
{
    GENERATED_BODY()
protected:
    virtual void BeginPlay() override;
private:
    void Assemble();
};
