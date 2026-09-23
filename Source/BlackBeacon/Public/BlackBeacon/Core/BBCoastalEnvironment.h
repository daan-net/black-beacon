// BLACK BEACON - opening coast presentation actor.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "BBCoastalEnvironment.generated.h"

class USceneComponent;
class UStaticMeshComponent;

UCLASS()
class ABBCoastalEnvironment : public AActor
{
	GENERATED_BODY()

public:
	ABBCoastalEnvironment();

protected:
	virtual void BeginPlay() override;

private:
	UStaticMeshComponent* AddShape(
		const TCHAR* Name,
		const TCHAR* MeshPath,
		const FVector& Location,
		const FRotator& Rotation,
		const FVector& Scale,
		bool bCollision);

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot = nullptr;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> RockSurfaces;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> PathSurfaces;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> WreckSurfaces;
};
