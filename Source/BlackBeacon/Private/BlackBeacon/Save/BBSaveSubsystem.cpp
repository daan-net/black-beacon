#include "BlackBeacon/Save/BBSaveSubsystem.h"

#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"
#include "EngineUtils.h"
#include "BlackBeacon/Power/BBGeneratorComponent.h"
#include "BlackBeacon/Lighthouse/BBLighthouseController.h"
#include "BlackBeacon/Lighthouse/BBLighthouseBeamComponent.h"
#include "BlackBeacon/Lighthouse/BBBeamRevealComponent.h"
#include "BlackBeacon/Objectives/BBObjectiveSystem.h"
#include "BlackBeacon/Weather/BBWeatherController.h"


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
		}
	}

	for (TActorIterator<ABBLighthouseController> It(World); It; ++It)
	{
		Data.bLighthousePowered = It->IsPowered();
		if (It->BeamComponent)
		{
			Data.bBeamRunning = It->BeamComponent->GetRotationMode() != EBBBeamRotationMode::Off;
			Data.BeamRotationMode = static_cast<int32>(It->BeamComponent->GetRotationMode());
		}
	}

	for (TActorIterator<ABBWeatherController> It(World); It; ++It)
	{
		Data.WeatherPhase = static_cast<float>(It->GetTargetPhase());
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
			if (Data.bGeneratorRunning)
			{
				Gen->Start();
			}
			else
			{
				Gen->Stop();
			}
		}
	}

	for (TActorIterator<ABBLighthouseController> It(World); It; ++It)
	{
		if (It->BeamComponent)
		{
			// The lighthouse controller itself might need to update its power state? 
			// Wait, the generator powers it automatically via the PowerNetwork.
			// But we should forcefully restore the beam rotation mode.
			It->BeamComponent->SetRotationMode(static_cast<EBBBeamRotationMode>(Data.BeamRotationMode));
		}
	}

	for (TActorIterator<ABBWeatherController> It(World); It; ++It)
	{
		It->SetWeather(static_cast<EBBWeatherPhase>(Data.WeatherPhase), 0.0f);
	}

	if (World->GetGameInstance())
	{
		if (UBBObjectiveSystem* Objectives = World->GetGameInstance()->GetSubsystem<UBBObjectiveSystem>())
		{
			Objectives->RestoreCompletedObjectives(Data.CompletedObjectives);
		}
	}

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		if (UBBBeamRevealComponent* Reveal = It->FindComponentByClass<UBBBeamRevealComponent>())
		{
			if (Data.PersistentlyRevealedActors.Contains(It->GetName()))
			{
				Reveal->ForceReveal();
			}
		}
	}
}
