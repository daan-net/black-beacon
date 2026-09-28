#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "BlackBeacon/Lighthouse/BBLighthouseController.h"
#include "BlackBeacon/Lighthouse/BBBeamRevealComponent.h"
#include "BlackBeacon/Power/BBGeneratorComponent.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBBEnvironmentKitReviewTest, "BlackBeacon.V052.EnvironmentKitReview",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FBBEnvironmentKitReviewTest::RunTest(const FString& Parameters)
{
    struct FReview
    {
        TWeakObjectPtr<ACameraActor> Camera;
        TWeakObjectPtr<AActor> PreviousView;
        TWeakObjectPtr<ABBLighthouseController> Lighthouse;
        TWeakObjectPtr<UBBBeamRevealComponent> Reveal;
        APlayerController* Controller = nullptr;
        FVector Target;
        double Start = FPlatformTime::Seconds();
        double Stage = Start;
        int32 Index = 0;
        bool Captured = false;
    };
    const TSharedRef<FReview> State = MakeShared<FReview>();
    ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, State]()
    {
        const double Now = FPlatformTime::Seconds();
        const auto Cleanup = [&]()
        {
            if (State->Controller) State->Controller->SetViewTarget(State->PreviousView.Get());
            if (State->Camera.IsValid()) State->Camera->Destroy();
        };
        if (Now - State->Start > 110)
        {
            AddError(TEXT("Environment kit review timed out"));
            Cleanup();
            return true;
        }
        if (!State->Camera.IsValid())
        {
            if (Now - State->Start < 5) return false;
            UWorld* World = nullptr;
            for (const FWorldContext& Context : GEngine->GetWorldContexts())
            {
                if (Context.WorldType == EWorldType::Game && Context.World() && Context.World()->HasBegunPlay())
                    World = Context.World();
            }
            if (!World || !World->GetFirstPlayerController()) return false;
            int32 Visuals = 0;
            for (TActorIterator<AActor> It(World); It; ++It)
            {
                if (ABBLighthouseController* Lighthouse = Cast<ABBLighthouseController>(*It)) State->Lighthouse = Lighthouse;
                if (UBBGeneratorComponent* Generator = It->FindComponentByClass<UBBGeneratorComponent>()) Generator->Start();
                for (UStaticMeshComponent* Mesh : TInlineComponentArray<UStaticMeshComponent*>(*It))
                {
                    if (Mesh->ComponentHasTag(TEXT("BB_EnvironmentKitVisual")))
                    {
                        ++Visuals;
                        TestTrue(TEXT("Scan instance resolves its imported mesh"), Mesh->GetStaticMesh() != nullptr);
                        TestEqual(TEXT("Environment kit never changes traversal collision"), Mesh->GetCollisionEnabled(), ECollisionEnabled::NoCollision);
                        TestFalse(TEXT("Environment kit never changes navigation"), Mesh->CanEverAffectNavigation());
                    }
                    if (It->ActorHasTag(TEXT("BB_Anomaly")) && Mesh->GetName() == TEXT("RevealedRuinPart_00"))
                    {
                        State->Target = Mesh->Bounds.Origin;
                        State->Reveal = It->FindComponentByClass<UBBBeamRevealComponent>();
                    }
                }
            }
            if (!TestTrue(TEXT("Lighthouse and real wreck reveal target exist"), State->Lighthouse.IsValid() && State->Reveal.IsValid())) return true;
            TestEqual(TEXT("Bounded environment kit placements assembled"), Visuals, 14);
            UStaticMeshComponent* Tower = State->Lighthouse->TowerExteriorSkin;
            const int32 Slot = Tower->GetMaterialIndex(TEXT("TowerPaint"));
            TestTrue(TEXT("Exterior scanned plaster override applied"), Slot != INDEX_NONE && Tower->GetMaterial(Slot)
                && Tower->GetMaterial(Slot)->GetName() == TEXT("MI_EK_ExteriorPlaster"));
            State->Controller = World->GetFirstPlayerController();
            State->PreviousView = State->Controller->GetViewTarget();
            State->Camera = World->SpawnActor<ACameraActor>();
            State->Controller->SetViewTarget(State->Camera.Get());
            State->Stage = Now;
        }
        const FVector Positions[] = {
            {1200,-420,210}, {-1900,-2300,1050}, {2950,-1350,200}, {-145,-730,150},
            {210,15,155}, {380,-75,1780}, {-2300,-1900,800}, {-2200,1000,1650}
        };
        const FVector Targets[] = {
            {0,0,780}, {0,0,1170}, {2300,-800,-80}, {30,-610,120},
            {80,-130,250}, {0,0,1900}, {0,0,1530}, State->Target
        };
        const TCHAR* Names[] = {
            TEXT("01_lighthouse_approach"), TEXT("02_lighthouse_exterior"), TEXT("03_shoreline"), TEXT("04_generator_room"),
            TEXT("05_stairwell"), TEXT("06_lantern_room"), TEXT("07_beacon_active"), TEXT("08_shipwreck_reveal")
        };
        State->Camera->SetActorLocation(Positions[State->Index]);
        State->Camera->SetActorRotation((Targets[State->Index] - Positions[State->Index]).Rotation());
        State->Camera->GetCameraComponent()->SetFieldOfView(State->Index == 5 ? 100 : 78);
        if (State->Index >= 6)
        {
            if (!State->Lighthouse->HasStartedBeam()) State->Lighthouse->RequestBeamStart();
            const FRotator Aim = (State->Target - State->Lighthouse->BeamComponent->GetComponentLocation()).Rotation();
            State->Lighthouse->BeamComponent->SetManualYawTarget(Aim.Yaw);
            State->Lighthouse->BeamComponent->SetManualPitchDegrees(Aim.Pitch);
        }
        if (!State->Captured)
        {
            if (Now - State->Stage < 6) return false;
            const FString Capture = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()
                / TEXT("EnvironmentKitV1/Captures") / FString::Printf(TEXT("%s.png"), Names[State->Index]));
            FScreenshotRequest::RequestScreenshot(Capture, false, false);
            State->Captured = true;
            State->Stage = Now;
            return false;
        }
        if (Now - State->Stage < .5) return false;
        ++State->Index;
        State->Captured = false;
        State->Stage = Now;
        if (State->Index < UE_ARRAY_COUNT(Positions)) return false;
        TestTrue(TEXT("Real beam query reveals wreck in rendered review"), State->Reveal->WasFullyRevealed() && State->Reveal->GetVisibilityAmount() > .95);
        Cleanup();
        return true;
    }));
    return true;
}
#endif
