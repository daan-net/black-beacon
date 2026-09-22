#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "UnrealClient.h"
#include "HAL/PlatformTime.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Camera/CameraComponent.h"
#include "Camera/CameraActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"

#include "BlackBeacon/Core/BBGameMode.h"
#include "BlackBeacon/Core/BBPlayerCharacter.h"
#include "BlackBeacon/Core/BBPlayerController.h"
#include "BlackBeacon/Interaction/BBInteractionComponent.h"
#include "BlackBeacon/Interaction/BBPromptWidget.h"
#include "BlackBeacon/Lighthouse/BBBeamRevealComponent.h"
#include "BlackBeacon/Lighthouse/BBLighthouseBeamComponent.h"
#include "BlackBeacon/Lighthouse/BBLighthouseController.h"
#include "BlackBeacon/Objectives/BBObjectiveSystem.h"
#include "BlackBeacon/Power/BBGeneratorComponent.h"
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
        TWeakObjectPtr<AActor> Anomaly;
        TWeakObjectPtr<APawn> Pawn;
        TWeakObjectPtr<UBBInteractionComponent> Interaction;
        TWeakObjectPtr<ACameraActor> CaptureCamera;
        int32 CaptureIndex = 0;
        FVector AirCameraPosition = FVector::ZeroVector;
        FVector AirCameraTarget = FVector::ZeroVector;
    };
    TSharedRef<FFlowState> State = MakeShared<FFlowState>();

    ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, State]()
    {
        const double Now = FPlatformTime::Seconds();
        if (Now - State->StartedAt > 25.0)
        {
            AddError(TEXT("Gameplay flow timed out"));
            return true;
        }

        if (State->Stage == 0)
        {
            UWorld* World = FindGameWorld();
            if (!World || !World->HasBegunPlay())
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
            TestNotNull(TEXT("Reveal component"), State->Anomaly->FindComponentByClass<UBBBeamRevealComponent>());
            TActorIterator<ABBWeatherController> WeatherIt(World);
            ABBWeatherController* Weather = WeatherIt ? *WeatherIt : nullptr;
            TestNotNull(TEXT("Weather controller"), Weather);
            if (Weather)
            {
                TestTrue(TEXT("Rain phase has fog density"), Weather->GetFogDensity() > 0.001f);
                TestTrue(TEXT("Volumetric fog is enabled"), Weather->FogComponent->bEnableVolumetricFog);
            }
            USpotLightComponent* BeamLight = State->Lighthouse->FindComponentByClass<USpotLightComponent>();
            TestNotNull(TEXT("Beam spotlight"), BeamLight);
            if (BeamLight)
            {
                TestTrue(TEXT("Beam contributes to volumetric fog"), BeamLight->VolumetricScatteringIntensity > 0.0f);
            }
            TestTrue(TEXT("Generator spawned in annex"), GeneratorActor->GetActorLocation().Equals(FVector(1560.0f, 1120.0f, 40.0f), 1.0f));
            TestTrue(TEXT("Anomaly spawned on far cliff"), State->Anomaly->GetActorLocation().Equals(FVector(-5200.0f, 4200.0f, 80.0f), 1.0f));
            TestTrue(TEXT("Player starts at landing"), FVector2D(State->Pawn->GetActorLocation()).Equals(FVector2D(-4500.0f, -600.0f), 10.0f));
            State->Pawn->SetActorLocation(FVector(360.0f, 0.0f, 100.0f));
            TestTrue(TEXT("Entering lighthouse volume completes objective"), Objectives->IsCompleted(TEXT("BB_OBJ_ENTER_LIGHTHOUSE")));
            State->Pawn->SetActorLocation(FVector(1560.0f, 1120.0f, 100.0f));
            TestTrue(TEXT("Entering annex volume finds generator"), Objectives->IsCompleted(TEXT("BB_OBJ_FIND_GENERATOR")));
            State->Pawn->SetActorLocation(FVector(1300.0f, 1120.0f, 100.0f));
            Character->GetController()->SetControlRotation((GeneratorActor->GetActorLocation() - Character->GetFirstPersonCamera()->GetComponentLocation()).Rotation());
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
            UStaticMeshComponent* ControlMesh = Lighthouse->FindComponentByClass<UStaticMeshComponent>();
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
                FVector(-5600.0f, 3900.0f, 500.0f)
            };
            const FVector Targets[] = {
                FVector(0.0f, 0.0f, 1700.0f),
                FVector(-700.0f, 600.0f, 1700.0f),
                State->Anomaly->GetActorLocation(),
                State->Anomaly->GetActorLocation()
            };
            if (State->CaptureIndex == 1)
            {
                const auto Query = Lighthouse->BeamComponent->GetBeamQuery();
                const FVector Origin(Query.Origin.X, Query.Origin.Y, Query.Origin.Z);
                const FVector Direction(Query.Direction.X, Query.Direction.Y, Query.Direction.Z);
                State->AirCameraTarget = Origin + Direction * 2000.0f;
                State->AirCameraPosition = State->AirCameraTarget
                    + FVector(-Direction.Y, Direction.X, 0.0f).GetSafeNormal() * 1500.0f
                    + FVector(0.0f, 0.0f, 150.0f);
            }
            const FVector Position = State->CaptureIndex == 1 ? State->AirCameraPosition : Positions[State->CaptureIndex];
            const FVector Target = State->CaptureIndex == 1 ? State->AirCameraTarget : Targets[State->CaptureIndex];
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
                TEXT("BlackBeacon_M01_D_Reveal.png")
            };
            FScreenshotRequest::RequestScreenshot(Names[State->CaptureIndex], false, false);
            ++State->CaptureIndex;
            State->Stage = 8;
            State->StageAt = Now;
            return false;
        }
        if (State->Stage == 8 && Now - State->StageAt >= 0.3)
        {
            State->Stage = State->CaptureIndex < 4 ? 5 : 7;
        }
        if (State->Stage == 7)
        {
            USpotLightComponent* BeamLight = Lighthouse->FindComponentByClass<USpotLightComponent>();
            Generator->Stop();
            TestFalse(TEXT("Stopping generator removes lighthouse power"), Lighthouse->IsPowered());
            TestFalse(TEXT("Beam turns off when power is lost"), Lighthouse->BeamComponent->IsPowered());
            TestTrue(TEXT("Beam light hides after power loss"), BeamLight && !BeamLight->IsVisible());
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
