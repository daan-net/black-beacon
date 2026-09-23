// BLACK BEACON - opening coast presentation actor.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "BBCoastalEnvironment.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UInstancedStaticMeshComponent;
class UBBGeneratorComponent;

UCLASS()
class ABBCoastalEnvironment : public AActor
{
	GENERATED_BODY()

public:
	ABBCoastalEnvironment();

protected:
	virtual void BeginPlay() override;

private:
	void BuildRevealedRuin();
	void BuildGeneratorMachinery();
	void HandleGeneratorRunningChanged(bool bRunning);
	void AdvanceGeneratorFlywheel();
	UStaticMeshComponent* AddShape(
		const TCHAR* Name,
		const TCHAR* MeshPath,
		const FVector& Location,
		const FRotator& Rotation,
		const FVector& Scale,
		bool bCollision);

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "BlackBeacon|Coast")
	TObjectPtr<UInstancedStaticMeshComponent> RockField = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "BlackBeacon|Coast")
	TObjectPtr<UStaticMeshComponent> OceanSurface = nullptr;

	UPROPERTY()
	TObjectPtr<USceneComponent> GeneratorFlywheelPivot = nullptr;

	UPROPERTY()
	TObjectPtr<UBBGeneratorComponent> GeneratorComponent = nullptr;

	FTimerHandle GeneratorFlywheelTimer;
	float GeneratorFlywheelSpeedDegrees = 0.0f;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> RockSurfaces;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> PathSurfaces;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> WreckSurfaces;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> AnnexSurfaces;
};
