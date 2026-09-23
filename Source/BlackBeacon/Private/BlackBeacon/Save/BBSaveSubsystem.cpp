#include "BlackBeacon/Save/BBSaveSubsystem.h"

#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "BlackBeacon/Power/BBGeneratorComponent.h"
#include "BlackBeacon/Lighthouse/BBLighthouseController.h"
#include "BlackBeacon/Lighthouse/BBLighthouseBeamComponent.h"
#include "BlackBeacon/Lighthouse/BBBeamRevealComponent.h"
#include "BlackBeacon/Objectives/BBObjectiveSystem.h"
#include "BlackBeacon/Weather/BBWeatherController.h"
#include "GameFramework/Pawn.h"


namespace
{
	constexpr const TCHAR* kDefaultSaveSlot = TEXT("slot_blackbeacon_0");
	FString ResolveSlotInternal(const FString& SlotName)
	{
		return SlotName.IsEmpty() ? FString(kDefaultSaveSlot) : SlotName;
	}
}

FString UBBSaveSubsystem::ResolveSlot(const FString& SlotName)
{
	return ResolveSlotInternal(SlotName);
}

bool UBBSaveSubsystem::SaveWorldData(const FBBWorldSaveData& WorldData, const FString& SlotName)
{
	UBBSaveGame* const Save = Cast<UBBSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UBBSaveGame::StaticClass()));
	if (!Save)
	{
		return false;
	}

	Save->WorldData = WorldData;
	return UGameplayStatics::SaveGameToSlot(Save, ResolveSlotInternal(SlotName), 0);
}

bool UBBSaveSubsystem::LoadWorldData(FBBWorldSaveData& OutWorldData, const FString& SlotName)
{
	USaveGame* const Loaded = UGameplayStatics::LoadGameFromSlot(ResolveSlotInternal(SlotName), 0);
	UBBSaveGame* const Save = Cast<UBBSaveGame>(Loaded);
	if (!Save)
	{
		return false;
	}

	OutWorldData = Save->WorldData;
	return true;
}

bool UBBSaveSubsystem::HasSaveData(const FString& SlotName) const
{
	return UGameplayStatics::DoesSaveGameExist(ResolveSlotInternal(SlotName), 0);
}

FBBWorldSaveData UBBSaveSubsystem::BuildSnapshot(UWorld* World)
{
	FBBWorldSaveData Data;
	if (!World) return Data;

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		if (UBBGeneratorComponent* Gen = It->FindComponentByClass<UBBGeneratorComponent>())
		{
			Data.bGeneratorRunning = Gen->IsRunning();
			Data.GeneratorSpinUpProgress = Gen->GetSpinUpProgress();
			Data.bGeneratorHasProducedOnce = Gen->HasProducedOnce();
		}
	}

	for (TActorIterator<ABBLighthouseController> It(World); It; ++It)
	{
		Data.bLighthousePowered = It->IsPowered();
		if (It->BeamComponent)
		{
			Data.bBeamRunning = It->BeamComponent->GetRotationMode() != EBBBeamRotationMode::Off;
			Data.bBeamStarted = It->HasStartedBeam();
			if (World->GetGameInstance())
			{
				if (UBBObjectiveSystem* Objectives = World->GetGameInstance()->GetSubsystem<UBBObjectiveSystem>())
				{
					Data.bAimObjectiveCompleted = Objectives->IsCompleted(TEXT("BB_OBJ_AIM_BEAM"));
				}
			}
			Data.BeamRotationMode = static_cast<int32>(It->BeamComponent->GetRotationMode());
			Data.BeamYawDegrees = It->BeamComponent->GetCurrentYawDegrees();
			Data.BeamPitchDegrees = It->BeamComponent->GetCurrentPitchDegrees();
		}
	}

	for (TActorIterator<ABBWeatherController> It(World); It; ++It)
	{
		Data.WeatherPhase = static_cast<int32>(It->GetTargetPhase());
	}

	if (APlayerController* const PlayerController = World->GetFirstPlayerController())
	{
		if (APawn* const Pawn = PlayerController->GetPawn())
		{
			Data.bHasPlayerTransform = true;
			Data.PlayerLocation = Pawn->GetActorLocation();
			Data.PlayerRotation = Pawn->GetActorRotation();
		}
	}

	if (World->GetGameInstance())
	{
		if (UBBObjectiveSystem* Objectives = World->GetGameInstance()->GetSubsystem<UBBObjectiveSystem>())
		{
			Data.CompletedObjectives = Objectives->GetCompletedObjectives();
		}
	}

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		if (UBBBeamRevealComponent* Reveal = It->FindComponentByClass<UBBBeamRevealComponent>())
		{
			if (Reveal->WasFullyRevealed() && Reveal->bPersistent)
			{
				Data.PersistentlyRevealedActors.Add(It->GetName());
			}
		}
	}

	return Data;
}

void UBBSaveSubsystem::RestoreSnapshot(UWorld* World, const FBBWorldSaveData& Data)
{
	if (!World) return;

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		if (UBBGeneratorComponent* Gen = It->FindComponentByClass<UBBGeneratorComponent>())
		{
			Gen->RestoreState(
				Data.bGeneratorRunning,
				Data.GeneratorSpinUpProgress,
				Data.bGeneratorHasProducedOnce);
		}
	}

	for (TActorIterator<ABBLighthouseController> It(World); It; ++It)
	{
		It->RestoreBeamState(
			Data.bBeamStarted,
			Data.bAimObjectiveCompleted,
			static_cast<EBBBeamRotationMode>(Data.BeamRotationMode),
			Data.BeamYawDegrees,
			Data.BeamPitchDegrees);
	}

	for (TActorIterator<ABBWeatherController> It(World); It; ++It)
	{
		const int32 PhaseIndex = FMath::Clamp(Data.WeatherPhase, 0, static_cast<int32>(EBBWeatherPhase::Storm));
		It->SetWeather(static_cast<EBBWeatherPhase>(PhaseIndex), 0.0f);
	}

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		if (UBBBeamRevealComponent* Reveal = It->FindComponentByClass<UBBBeamRevealComponent>())
		{
			if (Reveal->bPersistent)
			{
				Reveal->SetRevealedForRestore(Data.PersistentlyRevealedActors.Contains(It->GetName()));
			}
		}
	}

	if (World->GetGameInstance())
	{
		if (UBBObjectiveSystem* Objectives = World->GetGameInstance()->GetSubsystem<UBBObjectiveSystem>())
		{
			Objectives->RestoreCompletedObjectives(Data.CompletedObjectives);
		}
	}

	if (Data.bHasPlayerTransform)
	{
		if (APlayerController* const PlayerController = World->GetFirstPlayerController())
		{
			if (APawn* const Pawn = PlayerController->GetPawn())
			{
				Pawn->SetActorLocationAndRotation(Data.PlayerLocation, Data.PlayerRotation, false, nullptr, ETeleportType::TeleportPhysics);
			}
		}
	}
}
