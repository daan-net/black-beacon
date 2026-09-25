#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BBHeroArchitectureComponent.generated.h"

// Visual migration for the saved slice. Existing actors retain gameplay authority.
UCLASS(config=Game)
class UBBHeroArchitectureComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UBBHeroArchitectureComponent();
    UPROPERTY(config, EditAnywhere, Category="Presentation") float PracticalLumens = 75.0f;
    UPROPERTY(config, EditAnywhere, Category="Presentation") float PracticalRadiusCm = 330.0f;
    UPROPERTY(config, EditAnywhere, Category="Presentation") float AnnexPracticalLumens = 100.0f;
    UPROPERTY(config, EditAnywhere, Category="Presentation") FVector AnnexPracticalOffset = FVector(-65.0f, -30.0f, -18.0f);
protected:
    virtual void BeginPlay() override;
private:
    void Assemble();
};
