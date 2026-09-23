#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "UnrealClient.h"
#include "HAL/PlatformTime.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "Camera/CameraActor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"

#include "BlackBeacon/Core/BBGameMode.h"
#include "BlackBeacon/Core/BBCoastalEnvironment.h"
#include "BlackBeacon/Core/BBPlayerCharacter.h"
#include "BlackBeacon/Core/BBPlayerController.h"
#include "BlackBeacon/Interaction/BBInteractionComponent.h"
#include "BlackBeacon/Interaction/BBPromptWidget.h"
#include "BlackBeacon/Lighthouse/BBBeamRevealComponent.h"
#include "BlackBeacon/Lighthouse/BBLighthouseBeamComponent.h"
#include "BlackBeacon/Lighthouse/BBLighthouseController.h"
#include "BlackBeacon/Objectives/BBObjectiveSystem.h"
#include "BlackBeacon/Power/BBGeneratorComponent.h"
#include "BlackBeacon/Save/BBSaveSubsystem.h"
#include "BlackBeacon/Weather/BBWeatherController.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBBGameplayFlowTest, "BlackBeacon.M01.GameplayFlow",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

namespace
{
    UWorld* FindGameWorld()
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.WorldType == EWorldType::Game && Context.World())
            {
                return Context.World();
            }
        }
        return nullptr;
    }

    AActor* FindTaggedActor(UWorld* World, FName Tag)
    {
        for (TActorIterator<AActor> It(World); It; ++It)
        {
            if (It->ActorHasTag(Tag))
            {
                return *It;
            }
        }
        return nullptr;
    }

}

bool FBBGameplayFlowTest::RunTest(const FString& Parameters)
{
    struct FFlowState
    {
        int32 Stage = 0;
        double StartedAt = FPlatformTime::Seconds();
        double StageAt = StartedAt;
        TWeakObjectPtr<UWorld> World;
        TWeakObjectPtr<UBBObjectiveSystem> Objectives;
        TWeakObjectPtr<UBBGeneratorComponent> Generator;
        TWeakObjectPtr<ABBLighthouseController> Lighthouse;
        TWeakObjectPtr<UInstancedStaticMeshComponent> LanternFresnelBands;
        TArray<TWeakObjectPtr<UStaticMeshComponent>> LanternGlazingPanels;
        TWeakObjectPtr<AActor> Anomaly;
        TWeakObjectPtr<APawn> Pawn;
        TWeakObjectPtr<UBBInteractionComponent> Interaction;
        TWeakObjectPtr<ABBWeatherController> Weather;
        FVector RainFieldAnchor = FVector::ZeroVector;
        TWeakObjectPtr<ACameraActor> CaptureCamera;
        int32 CaptureIndex = 0;
        FVector AirCameraPosition = FVector::ZeroVector;
        FVector AirCameraTarget = FVector::ZeroVector;
        bool bOpeningCaptured = false;
        FString SaveSlot = TEXT("BB_M02_Automation_Restore");
    };
    TSharedRef<FFlowState> State = MakeShared<FFlowState>();

    ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, State]()
    {
        const double Now = FPlatformTime::Seconds();
        // The rendered suite captures eight 1080p views after the real flow;
        // shader warm-up and GPU readbacks can exceed the logic-only budget.
        if (Now - State->StartedAt > 120.0)
        {
            const AActor* FocusedActor = State->Interaction.IsValid()
                ? State->Interaction->GetFocusedActor()
                : nullptr;
            AddError(FString::Printf(TEXT("Gameplay flow timed out in stage %d; focused actor: %s"),
                State->Stage, *GetNameSafe(FocusedActor)));
            return true;
        }

        if (State->Stage == 0)
        {
            UWorld* World = FindGameWorld();
            if (!World || !World->HasBegunPlay())
            {
                return false;
            }
            if (FApp::CanEverRender() && !State->bOpeningCaptured)
            {
                if (Now - State->StartedAt < 3.0)
                {
                    return false;
                }
                FScreenshotRequest::RequestScreenshot(TEXT("BlackBeacon_M02_Opening.png"), false, false);
                State->bOpeningCaptured = true;
                State->StageAt = Now;
                return false;
            }
            if (State->bOpeningCaptured && Now - State->StageAt < 0.3)
            {
                return false;
            }
            State->World = World;
            State->Pawn = World->GetFirstPlayerController() ? World->GetFirstPlayerController()->GetPawn() : nullptr;
            TestNotNull(TEXT("Player pawn"), State->Pawn.Get());
            ABBlackBeaconPlayerCharacter* Character = Cast<ABBlackBeaconPlayerCharacter>(State->Pawn.Get());
            State->Interaction = Character ? Character->GetInteractionComponent() : nullptr;
            TestNotNull(TEXT("Player interaction component"), State->Interaction.Get());
            TestNotNull(TEXT("BlackBeacon GameMode"), Cast<ABBlackBeaconGameMode>(World->GetAuthGameMode()));
            TActorIterator<ABBCoastalEnvironment> CoastIt(World);
            ABBCoastalEnvironment* Coast = CoastIt ? *CoastIt : nullptr;
            TestNotNull(TEXT("Opening coast environment"), Coast);
            if (Coast)
            {
                TestTrue(TEXT("Opening coast uses the map origin"), Coast->GetActorLocation().IsNearlyZero(1.0f));
                UInstancedStaticMeshComponent* RockField = Coast->FindComponentByClass<UInstancedStaticMeshComponent>();
                TestNotNull(TEXT("Coastal boulder field"), RockField);
                if (RockField)
                {
                    TestEqual(TEXT("Coastal boulders block the player"),
                        RockField->GetCollisionResponseToChannel(ECC_Pawn), ECR_Block);
                    TestTrue(TEXT("Coastal boulders have collision enabled"),
                        RockField->GetCollisionEnabled() == ECollisionEnabled::QueryAndPhysics);
                    TestTrue(TEXT("Coastal rock field uses clustered lobe shapes"),
                        RockField->GetInstanceCount() > 150);
                }

                TArray<UStaticMeshComponent*> CoastMeshes;
                Coast->GetComponents<UStaticMeshComponent>(CoastMeshes);
                int32 BlockingWreckageCount = 0;
                int32 BlockingAnnexCount = 0;
                bool bHasNearDoorFrame = false;
                bool bHasFarDoorFrame = false;
                for (const UStaticMeshComponent* Mesh : CoastMeshes)
                {
                    if (Mesh && Mesh->GetName().StartsWith(TEXT("Wreck_"))
                        && Mesh->GetCollisionEnabled() == ECollisionEnabled::QueryAndPhysics
                        && Mesh->GetCollisionResponseToChannel(ECC_Pawn) == ECR_Block)
                    {
                        ++BlockingWreckageCount;
                    }
                    if (Mesh && Mesh->GetName().StartsWith(TEXT("Annex"))
                        && Mesh->GetCollisionEnabled() == ECollisionEnabled::QueryAndPhysics
                        && Mesh->GetCollisionResponseToChannel(ECC_Pawn) == ECR_Block)
                    {
                        ++BlockingAnnexCount;
                    }
                    bHasNearDoorFrame |= Mesh && Mesh->GetName() == TEXT("AnnexWestLeft");
                    bHasFarDoorFrame |= Mesh && Mesh->GetName() == TEXT("AnnexWestRight");
                }
                TestTrue(TEXT("Shipwreck debris blocks the player"), BlockingWreckageCount > 0);
                TestTrue(TEXT("Generator annex has blocking walls and roof"), BlockingAnnexCount >= 8);
                TestTrue(TEXT("Generator annex has both sides of an open doorway"), bHasNearDoorFrame && bHasFarDoorFrame);
            }
            UGameInstance* GameInstance = World->GetGameInstance();
            UBBObjectiveSystem* Objectives = GameInstance ? GameInstance->GetSubsystem<UBBObjectiveSystem>() : nullptr;
            TestNotNull(TEXT("Objective subsystem"), Objectives);
            if (!Objectives)
            {
                return true;
            }
            State->Objectives = Objectives;
            TestTrue(TEXT("Arrival completed"), Objectives->IsCompleted(TEXT("BB_OBJ_ARRIVE")));
            TestEqual(TEXT("Only enter objective is current"), Objectives->GetCurrentObjectiveId(), FString(TEXT("BB_OBJ_ENTER_LIGHTHOUSE")));
            TestFalse(TEXT("Generator objective is still locked"), Objectives->IsActive(TEXT("BB_OBJ_START_GENERATOR")));

            AActor* GeneratorActor = FindTaggedActor(World, TEXT("BB_Generator"));
            State->Generator = GeneratorActor ? GeneratorActor->FindComponentByClass<UBBGeneratorComponent>() : nullptr;
            TActorIterator<ABBLighthouseController> LighthouseIt(World);
            State->Lighthouse = LighthouseIt ? *LighthouseIt : nullptr;
            State->Anomaly = FindTaggedActor(World, TEXT("BB_Anomaly"));
            TestNotNull(TEXT("Generator component"), State->Generator.Get());
            TestNotNull(TEXT("Lighthouse controller"), State->Lighthouse.Get());
            TestNotNull(TEXT("Reveal anomaly"), State->Anomaly.Get());
            if (!State->Generator.IsValid() || !State->Lighthouse.IsValid() || !State->Anomaly.IsValid() || !State->Pawn.IsValid() || !State->Interaction.IsValid())
            {
                return true;
            }
            for (UStaticMeshComponent* Mesh : TInlineComponentArray<UStaticMeshComponent*>(State->Lighthouse.Get()))
            {
                if (Mesh && Mesh->GetName().StartsWith(TEXT("LanternGlass_")))
                {
                    State->LanternGlazingPanels.Add(Mesh);
                }
            }
            TestEqual(TEXT("Lantern housing has eight glazing panels"), State->LanternGlazingPanels.Num(), 8);
            UInstancedStaticMeshComponent* FresnelBands = nullptr;
            for (UInstancedStaticMeshComponent* Mesh : TInlineComponentArray<UInstancedStaticMeshComponent*>(State->Lighthouse.Get()))
            {
                if (Mesh && Mesh->GetName() == TEXT("LanternFresnelBands"))
                {
                    FresnelBands = Mesh;
                    break;
                }
            }
            TestNotNull(TEXT("Lantern has instanced Fresnel bands"), FresnelBands);
            if (FresnelBands)
            {
                TestEqual(TEXT("Fresnel bands detail all eight glass faces"), FresnelBands->GetInstanceCount(), 40);
                TestFalse(TEXT("Fresnel emission stays off before beam startup"), FresnelBands->IsVisible());
                State->LanternFresnelBands = FresnelBands;
            }
            int32 VisibleGlazingPanels = 0;
            for (const TWeakObjectPtr<UStaticMeshComponent>& Panel : State->LanternGlazingPanels)
            {
                VisibleGlazingPanels += Panel.IsValid() && Panel->IsVisible() ? 1 : 0;
                TestNotNull(TEXT("Unpowered glazing has a material"), Panel.IsValid() ? Panel->GetMaterial(0) : nullptr);
            }
            TestEqual(TEXT("Unpowered lantern glazing remains visible"), VisibleGlazingPanels, 8);
            UBBSaveSubsystem* SaveSubsystem = GameInstance->GetSubsystem<UBBSaveSubsystem>();
            TestNotNull(TEXT("Save subsystem"), SaveSubsystem);
            if (!SaveSubsystem || !SaveSubsystem->SaveWorldData(
                UBBSaveSubsystem::BuildSnapshot(World), State->SaveSlot))
            {
                AddError(TEXT("Could not write initial save snapshot"));
                return true;
            }
            TestNotNull(TEXT("Reveal component"), State->Anomaly->FindComponentByClass<UBBBeamRevealComponent>());
            TArray<UStaticMeshComponent*> AnomalyMeshes;
            State->Anomaly->GetComponents<UStaticMeshComponent>(AnomalyMeshes);
            TestTrue(TEXT("Anomaly has a modular ruin silhouette"), AnomalyMeshes.Num() >= 8);
            TActorIterator<ABBWeatherController> WeatherIt(World);
            ABBWeatherController* Weather = WeatherIt ? *WeatherIt : nullptr;
            TestNotNull(TEXT("Weather controller"), Weather);
            if (Weather)
            {
                State->Weather = Weather;
                State->RainFieldAnchor = Weather->RainRoot->GetComponentLocation();
                TestTrue(TEXT("Rain phase has fog density"), Weather->GetFogDensity() > 0.001f);
                TestTrue(TEXT("Volumetric fog is enabled"), Weather->FogComponent->bEnableVolumetricFog);
                TestTrue(TEXT("Rain field anchor matches the weather actor, not the player"),
                    State->RainFieldAnchor.Equals(Weather->GetActorLocation(), 1.0f));
            }
            USpotLightComponent* BeamLight = State->Lighthouse->FindComponentByClass<USpotLightComponent>();
            TestNotNull(TEXT("Beam spotlight"), BeamLight);
            if (BeamLight)
            {
                TestTrue(TEXT("Beam contributes to volumetric fog"), BeamLight->VolumetricScatteringIntensity > 0.0f);
            }
            TestTrue(TEXT("Generator spawned at usable height in annex"), GeneratorActor->GetActorLocation().Equals(FVector(1560.0f, 1120.0f, 90.0f), 1.0f));
            TestTrue(TEXT("Anomaly spawned on far cliff"), State->Anomaly->GetActorLocation().Equals(FVector(-5200.0f, 4200.0f, 80.0f), 1.0f));
            TestTrue(TEXT("Player starts at landing"), FVector2D(State->Pawn->GetActorLocation()).Equals(FVector2D(-4500.0f, -600.0f), 10.0f));
            State->Pawn->SetActorLocation(FVector(360.0f, 0.0f, 100.0f));
            TestTrue(TEXT("Entering lighthouse volume completes objective"), Objectives->IsCompleted(TEXT("BB_OBJ_ENTER_LIGHTHOUSE")));
            State->Pawn->SetActorLocation(FVector(1560.0f, 1120.0f, 100.0f));
            TestTrue(TEXT("Entering annex volume finds generator"), Objectives->IsCompleted(TEXT("BB_OBJ_FIND_GENERATOR")));
            State->Pawn->SetActorLocation(FVector(1300.0f, 1120.0f, 100.0f));
            const FVector CameraLocation = Character->GetFirstPersonCamera()->GetComponentLocation();
            const FVector NaturalAimTarget(GeneratorActor->GetActorLocation().X, GeneratorActor->GetActorLocation().Y, CameraLocation.Z);
            Character->GetController()->SetControlRotation((NaturalAimTarget - CameraLocation).Rotation());
            TestEqual(TEXT("Generator start is current"), Objectives->GetCurrentObjectiveId(), FString(TEXT("BB_OBJ_START_GENERATOR")));
            State->Stage = 1;
            State->StageAt = Now;
            return false;
        }

        UBBObjectiveSystem* Objectives = State->Objectives.Get();
        UBBGeneratorComponent* Generator = State->Generator.Get();
        ABBLighthouseController* Lighthouse = State->Lighthouse.Get();
        UWorld* World = State->World.Get();
        if (!Objectives || !Generator || !Lighthouse || !World)
        {
            AddError(TEXT("Gameplay objects were destroyed during flow"));
            return true;
        }
        if (State->Stage == 1)
        {
            if (State->Interaction->GetFocusedActor() != Generator->GetOwner())
            {
                return false;
            }
            ABBlackBeaconPlayerController* PlayerController = Cast<ABBlackBeaconPlayerController>(World->GetFirstPlayerController());
            TestNotNull(TEXT("Prompt widget"), PlayerController ? PlayerController->PromptWidget.Get() : nullptr);
            if (PlayerController && PlayerController->PromptWidget)
            {
                TestEqual(TEXT("Generator focus prompt"), PlayerController->PromptWidget->GetCurrentPrompt().ToString(), FString(TEXT("Start Generator")));
            }
            TestTrue(TEXT("Player trace interacts with generator"), State->Interaction->TryInteract());
            TestTrue(TEXT("Generator is spinning up"), Generator->IsRunning());
            if (PlayerController && PlayerController->PromptWidget)
            {
                TestEqual(TEXT("Generator prompt refreshes after interaction"), PlayerController->PromptWidget->GetCurrentPrompt().ToString(), FString(TEXT("Stop Generator")));
            }
            State->Stage = 2;
            State->StageAt = Now;
            return false;
        }
        if (State->Stage == 2)
        {
            if (!Generator->IsProducing())
            {
                TestFalse(TEXT("Power stays off during spin-up"), Lighthouse->IsPowered());
                return false;
            }
            TestTrue(TEXT("Generator start objective completed"), Objectives->IsCompleted(TEXT("BB_OBJ_START_GENERATOR")));
            TestTrue(TEXT("Restore power objective completed"), Objectives->IsCompleted(TEXT("BB_OBJ_RESTORE_POWER")));
            TestTrue(TEXT("Lighthouse has power"), Lighthouse->IsPowered());
            TestFalse(TEXT("Beam remains off before control interaction"), Lighthouse->BeamComponent->IsPowered());
            State->Pawn->SetActorLocation(FVector(-100.0f, 0.0f, 1650.0f));
            TestTrue(TEXT("Lantern volume completes climb objective"), Objectives->IsCompleted(TEXT("BB_OBJ_CLIMB")));
            ABBlackBeaconPlayerCharacter* Character = Cast<ABBlackBeaconPlayerCharacter>(State->Pawn.Get());
            UStaticMeshComponent* ControlMesh = nullptr;
            for (UStaticMeshComponent* Mesh : TInlineComponentArray<UStaticMeshComponent*>(Lighthouse))
            {
                if (Mesh && Mesh->GetName() == TEXT("ControlMesh"))
                {
                    ControlMesh = Mesh;
                    break;
                }
            }
            TestNotNull(TEXT("Traceable lantern control"), ControlMesh);
            if (!Character || !ControlMesh)
            {
                return true;
            }
            Character->GetController()->SetControlRotation((ControlMesh->GetComponentLocation() - Character->GetFirstPersonCamera()->GetComponentLocation()).Rotation());
            State->Stage = 3;
            State->StageAt = Now;
            return false;
        }
        if (State->Stage == 3)
        {
            if (State->Interaction->GetFocusedActor() != Lighthouse)
            {
                return false;
            }
            TestTrue(TEXT("Player trace starts lighthouse"), State->Interaction->TryInteract());
            TestTrue(TEXT("Beam starts through lighthouse interaction"), Lighthouse->BeamComponent->IsPowered());
            int32 VisibleGlazingPanels = 0;
            for (const TWeakObjectPtr<UStaticMeshComponent>& Panel : State->LanternGlazingPanels)
            {
                VisibleGlazingPanels += Panel.IsValid() && Panel->IsVisible() ? 1 : 0;
            }
            TestEqual(TEXT("Lantern glazing remains visible when lit"), VisibleGlazingPanels, 8);
            UInstancedStaticMeshComponent* FresnelBands = State->LanternFresnelBands.Get();
            TestNotNull(TEXT("Powered lighthouse Fresnel assembly"), FresnelBands);
            TestTrue(TEXT("Fresnel bands illuminate with the beam"), FresnelBands && FresnelBands->IsVisible());
            ABBlackBeaconPlayerController* PlayerController = Cast<ABBlackBeaconPlayerController>(World->GetFirstPlayerController());
            if (PlayerController && PlayerController->PromptWidget)
            {
                TestEqual(TEXT("Beam prompt refreshes after start"), PlayerController->PromptWidget->GetCurrentPrompt().ToString(), FString(TEXT("Take Control of the Beam")));
            }
            TestTrue(TEXT("Player trace takes beam control"), State->Interaction->TryInteract());
            if (PlayerController && PlayerController->PromptWidget)
            {
                TestEqual(TEXT("Beam prompt refreshes in manual mode"), PlayerController->PromptWidget->GetCurrentPrompt().ToString(), FString(TEXT("Release Beam Control")));
            }
            TestTrue(TEXT("Manual beam control active"), Lighthouse->IsBeamInManualMode());
            TestTrue(TEXT("Aim objective completed"), Objectives->IsCompleted(TEXT("BB_OBJ_AIM_BEAM")));
            const FVector ToAnomaly = State->Anomaly->GetActorLocation() - Lighthouse->BeamComponent->GetComponentLocation();
            Lighthouse->BeamComponent->SetManualYawTarget(FMath::RadiansToDegrees(FMath::Atan2(ToAnomaly.Y, ToAnomaly.X)));
            Lighthouse->BeamComponent->SetManualPitchDegrees(FMath::RadiansToDegrees(FMath::Atan2(ToAnomaly.Z, FVector2D(ToAnomaly.X, ToAnomaly.Y).Size())));
            State->Stage = 4;
            State->StageAt = Now;
            return false;
        }
        if (State->Stage == 4 && Objectives->IsCompleted(TEXT("BB_OBJ_DISCOVER_ANOMALY")))
        {
            TestTrue(TEXT("Full objective chain completed"), Objectives->IsFinished());
            TestFalse(TEXT("Anomaly is visible"), State->Anomaly->IsHidden());
            int32 VisibleRuinParts = 0;
            for (const UStaticMeshComponent* Mesh : TInlineComponentArray<UStaticMeshComponent*>(State->Anomaly.Get()))
            {
                if (Mesh && Mesh->GetName().StartsWith(TEXT("RevealedRuinPart_")) && Mesh->IsVisible())
                {
                    ++VisibleRuinParts;
                }
            }
            TestTrue(TEXT("BeamReveal exposes the ruin pieces"), VisibleRuinParts >= 7);
            USpotLightComponent* BeamLight = Lighthouse->FindComponentByClass<USpotLightComponent>();
            TestTrue(TEXT("Powered beam light is visible"), BeamLight && BeamLight->IsVisible());
            if (BeamLight)
            {
                TestTrue(TEXT("Powered beam has radiometric intensity"), BeamLight->Intensity > 1000.0f);
                const auto Query = Lighthouse->BeamComponent->GetBeamQuery();
                const FVector QueryDirection(Query.Direction.X, Query.Direction.Y, Query.Direction.Z);
                TestTrue(TEXT("Visible light matches gameplay direction"), FVector::DotProduct(BeamLight->GetForwardVector(), QueryDirection) > 0.999f);
            }
            if (FApp::CanEverRender())
            {
                State->CaptureCamera = World->SpawnActor<ACameraActor>();
                TestNotNull(TEXT("Fixed visual probe camera"), State->CaptureCamera.Get());
                if (!State->CaptureCamera.IsValid())
                {
                    return true;
                }
                World->GetFirstPlayerController()->SetViewTarget(State->CaptureCamera.Get());
                State->Stage = 5;
                State->StageAt = Now;
                return false;
            }
            State->Stage = 7;
        }
        if (State->Stage == 5)
        {
            const FVector Positions[] = {
                FVector(-1200.0f, -1800.0f, 900.0f),
                FVector(-1200.0f, 1000.0f, 1700.0f),
                FVector(-4000.0f, 3300.0f, 900.0f),
                FVector(-5600.0f, 3900.0f, 500.0f),
                FVector(-1200.0f, 1000.0f, 1700.0f), // Storm Dir 1
                FVector(-1200.0f, 1000.0f, 1700.0f), // Storm Dir 2
                FVector(-5600.0f, 3900.0f, 500.0f),  // Storm Indoors (Reveal pos)
                FVector(-1200.0f, 1000.0f, 1700.0f)  // Clear uses same position as B_Air
            };
            const FVector Targets[] = {
                FVector(0.0f, 0.0f, 1700.0f),
                FVector(-700.0f, 600.0f, 1700.0f),
                State->Anomaly->GetActorLocation(),
                State->Anomaly->GetActorLocation(),
                FVector(-700.0f, 600.0f, 1700.0f), // Storm Dir 1
                FVector(-1700.0f, 1400.0f, 1700.0f), // Storm Dir 2 (looking different way)
                State->Anomaly->GetActorLocation(), // Storm Indoors (looking at anomaly inside)
                FVector(-700.0f, 600.0f, 1700.0f)  // Clear
            };
            if (State->CaptureIndex == 1 || State->CaptureIndex == 4 || State->CaptureIndex == 5 || State->CaptureIndex == 7)
            {
                const auto Query = Lighthouse->BeamComponent->GetBeamQuery();
                const FVector Origin(Query.Origin.X, Query.Origin.Y, Query.Origin.Z);
                const FVector Direction(Query.Direction.X, Query.Direction.Y, Query.Direction.Z);
                State->AirCameraTarget = Origin + Direction * 2000.0f;
                State->AirCameraPosition = State->AirCameraTarget
                    + FVector(-Direction.Y, Direction.X, 0.0f).GetSafeNormal() * 1500.0f
                    + FVector(0.0f, 0.0f, 150.0f);
            }
            const FVector Position = (State->CaptureIndex == 1 || State->CaptureIndex == 4 || State->CaptureIndex == 5 || State->CaptureIndex == 7) ? State->AirCameraPosition : Positions[State->CaptureIndex];
            const FVector Target = (State->CaptureIndex == 1 || State->CaptureIndex == 4 || State->CaptureIndex == 5 || State->CaptureIndex == 7) ? State->AirCameraTarget : Targets[State->CaptureIndex];
            State->CaptureCamera->SetActorLocation(Position);
            State->CaptureCamera->SetActorRotation((Target - Position).Rotation());
            State->Stage = 6;
            State->StageAt = Now;
            return false;
        }
        if (State->Stage == 6 && Now - State->StageAt >= 1.0)
        {
            const TCHAR* Names[] = {
                TEXT("BlackBeacon_M01_A_Exterior.png"),
                TEXT("BlackBeacon_M01_B_Air.png"),
                TEXT("BlackBeacon_M01_C_Impact.png"),
                TEXT("BlackBeacon_M01_D_Reveal.png"),
                TEXT("BlackBeacon_M02_Storm_Dir1.png"),
                TEXT("BlackBeacon_M02_Storm_Dir2.png"),
                TEXT("BlackBeacon_M02_Storm_Indoors.png"),
                TEXT("BlackBeacon_M02_Clear.png")
            };
            FScreenshotRequest::RequestScreenshot(Names[State->CaptureIndex], false, false);
            ++State->CaptureIndex;
            State->Stage = 8;
            State->StageAt = Now;
            return false;
        }
        if (State->Stage == 8 && Now - State->StageAt >= 0.3)
        {
            if (State->CaptureIndex >= 4 && State->CaptureIndex <= 7)
            {
                TActorIterator<ABBWeatherController> WeatherIt(World);
                if (WeatherIt)
                {
                    EBBWeatherPhase Phase = EBBWeatherPhase::Storm;
                    if (State->CaptureIndex == 7) Phase = EBBWeatherPhase::Clear;
                    WeatherIt->SetWeather(Phase, 0.0f); // Instant
                }
                State->Stage = 5;
            }
            else
            {
                State->Stage = State->CaptureIndex < 8 ? 5 : 7;
            }
        }
        if (State->Stage == 7)
        {
            USpotLightComponent* BeamLight = Lighthouse->FindComponentByClass<USpotLightComponent>();
            Generator->Stop();
            TestFalse(TEXT("Stopping generator removes lighthouse power"), Lighthouse->IsPowered());
            TestFalse(TEXT("Beam turns off when power is lost"), Lighthouse->BeamComponent->IsPowered());
            int32 VisibleGlazingPanels = 0;
            for (const TWeakObjectPtr<UStaticMeshComponent>& Panel : State->LanternGlazingPanels)
            {
                VisibleGlazingPanels += Panel.IsValid() && Panel->IsVisible() ? 1 : 0;
                TestNotNull(TEXT("Glazing retains a material after power loss"), Panel.IsValid() ? Panel->GetMaterial(0) : nullptr);
            }
            TestEqual(TEXT("Lantern glazing remains visible after power loss"), VisibleGlazingPanels, 8);
            TestTrue(TEXT("Beam light hides after power loss"), BeamLight && !BeamLight->IsVisible());
            TestTrue(TEXT("Rain field stays fixed as the player traverses the map"),
                State->Weather.IsValid()
                && State->Weather->RainRoot->GetComponentLocation().Equals(State->RainFieldAnchor, 1.0f));

            UBBSaveSubsystem* SaveSubsystem = World->GetGameInstance()
                ? World->GetGameInstance()->GetSubsystem<UBBSaveSubsystem>() : nullptr;
            FBBWorldSaveData InitialSnapshot;
            TestNotNull(TEXT("Save subsystem survives the flow"), SaveSubsystem);
            TestTrue(TEXT("Initial checkpoint loads"),
                SaveSubsystem && SaveSubsystem->LoadWorldData(InitialSnapshot, State->SaveSlot));
            if (SaveSubsystem)
            {
                SaveSubsystem->RestoreSnapshot(World, InitialSnapshot);
                TestFalse(TEXT("Load restores the stopped generator"), Generator->IsRunning());
                TestFalse(TEXT("Load restores the unpowered lighthouse"), Lighthouse->IsPowered());
                TestFalse(TEXT("Load restores the stopped beam"), Lighthouse->BeamComponent->IsPowered());
                TestTrue(TEXT("Load restores the earlier objective"),
                    Objectives->GetCurrentObjectiveId() == TEXT("BB_OBJ_ENTER_LIGHTHOUSE"));
                TestTrue(TEXT("Load hides the not-yet-revealed anomaly"), State->Anomaly->IsHidden());
                TestTrue(TEXT("Load restores the player to the shore"),
                    FVector2D(State->Pawn->GetActorLocation()).Equals(FVector2D(-4500.0f, -600.0f), 10.0f));
            }
            UGameplayStatics::DeleteGameInSlot(State->SaveSlot, 0);
            if (State->CaptureCamera.IsValid())
            {
                State->CaptureCamera->SetActorLocation(State->AirCameraPosition);
                State->CaptureCamera->SetActorRotation((State->AirCameraTarget - State->AirCameraPosition).Rotation());
                State->Stage = 9;
                State->StageAt = Now;
                return false;
            }
            return true;
        }
        if (State->Stage == 9 && Now - State->StageAt >= 1.0)
        {
            FScreenshotRequest::RequestScreenshot(TEXT("BlackBeacon_M01_B_AirOff.png"), false, false);
            State->Stage = 10;
            State->StageAt = Now;
            return false;
        }
        if (State->Stage == 10 && Now - State->StageAt >= 0.3)
        {
            return true;
        }
        return false;
    }));
    return true;
}

#endif
