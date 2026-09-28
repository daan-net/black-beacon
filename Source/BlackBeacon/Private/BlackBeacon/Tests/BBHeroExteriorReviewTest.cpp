#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "UnrealClient.h"
#include "BlackBeacon/Lighthouse/BBLighthouseController.h"
#include "BlackBeacon/Power/BBGeneratorComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBBHeroExteriorReviewTest, "BlackBeacon.HeroExterior.Review",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FBBHeroExteriorReviewTest::RunTest(const FString& Parameters)
{
    struct FReview
    {
        TWeakObjectPtr<ACameraActor> Camera;
        TWeakObjectPtr<AActor> PreviousView;
        TWeakObjectPtr<ABBLighthouseController> Lighthouse;
        APlayerController* Controller = nullptr;
        double Start = FPlatformTime::Seconds();
        double Stage = Start;
        int32 Index = 0;
        bool Captured = false;
    };
    TSharedRef<FReview> State = MakeShared<FReview>();
    ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, State]()
    {
        const double Now = FPlatformTime::Seconds();
        if (Now-State->Start > 75)
        {
            AddError(TEXT("Exterior review timed out"));
            if (State->Controller) State->Controller->SetViewTarget(State->PreviousView.Get());
            if (State->Camera.IsValid()) State->Camera->Destroy();
            return true;
        }
        if (!State->Camera.IsValid())
        {
            if (Now-State->Start < 5) return false;
            UWorld* World = nullptr;
            for (const FWorldContext& Context : GEngine->GetWorldContexts())
                if (Context.WorldType == EWorldType::Game && Context.World() && Context.World()->HasBegunPlay()) World = Context.World();
            if (!World || !World->GetFirstPlayerController()) return false;
            ABBLighthouseController* Lighthouse = nullptr;
            for (TActorIterator<ABBLighthouseController> It(World); It; ++It) Lighthouse = *It;
            if (!Lighthouse) { AddError(TEXT("Missing lighthouse")); return true; }
            State->Lighthouse = Lighthouse;
            for (TActorIterator<AActor> It(World); It; ++It)
                if (UBBGeneratorComponent* Generator = It->FindComponentByClass<UBBGeneratorComponent>()) Generator->Start();
            int32 Modules = 0;
            for (UStaticMeshComponent* Mesh : TInlineComponentArray<UStaticMeshComponent*>(Lighthouse))
            {
                if (!Mesh->GetName().StartsWith(TEXT("HeroExterior"))) continue;
                ++Modules;
                TestTrue(TEXT("Exterior module resolves its imported mesh"), Mesh->GetStaticMesh() != nullptr);
                TestEqual(TEXT("Exterior cannot change traversal collision"), Mesh->GetCollisionEnabled(), ECollisionEnabled::NoCollision);
                TestTrue(TEXT("Exterior uses tower-foot coordinates"), Mesh->GetComponentLocation().Equals(FVector::ZeroVector, .1));
            }
            TestEqual(TEXT("All three exterior modules assembled"), Modules, 3);
            const int32 Slot = Lighthouse->TowerExteriorSkin->GetMaterialIndex(TEXT("TowerPaint"));
            TestTrue(TEXT("Only exterior paint receives new finish"), Slot != INDEX_NONE && Lighthouse->TowerExteriorSkin->GetMaterial(Slot) &&
                Lighthouse->TowerExteriorSkin->GetMaterial(Slot)->GetName() == TEXT("MI_EK_ExteriorPlaster"));
            State->Controller = World->GetFirstPlayerController();
            State->PreviousView = State->Controller->GetViewTarget();
            State->Camera = World->SpawnActor<ACameraActor>();
            State->Controller->SetViewTarget(State->Camera.Get());
            State->Stage = Now;
        }
        if (Now-State->Stage > 3 && State->Index == 0 && !State->Lighthouse->HasStartedBeam())
            State->Lighthouse->RequestBeamStart();
        // First two views match the existing V0.5 captures exactly for comparison.
        const FVector Positions[] = {{-4200,-2900,700},{-1900,-2300,1050},{850,-750,190},{800,-1000,1330},{650,50,180}};
        const FVector Targets[] = {{0,0,1550},{0,0,1170},{170,0,180},{0,0,1500},{280,0,165}};
        const TCHAR* Names[] = {TEXT("A_FullLighthouse"),TEXT("B_HeroThreeQuarter"),TEXT("C_Base"),TEXT("D_GalleryStructure"),TEXT("E_Doorway")};
        State->Camera->SetActorLocation(Positions[State->Index]);
        State->Camera->SetActorRotation((Targets[State->Index]-Positions[State->Index]).Rotation());
        State->Camera->GetCameraComponent()->SetFieldOfView(78);
        if (!State->Captured)
        {
            if (Now-State->Stage < 5) return false;
            FScreenshotRequest::RequestScreenshot(FString::Printf(TEXT("BlackBeacon_HeroExterior_%s.png"), Names[State->Index]), false, false);
            State->Captured = true;
            State->Stage = Now;
            return false;
        }
        if (Now-State->Stage < .5) return false;
        ++State->Index;
        State->Captured = false;
        State->Stage = Now;
        if (State->Index < UE_ARRAY_COUNT(Positions)) return false;
        State->Controller->SetViewTarget(State->PreviousView.Get());
        State->Camera->Destroy();
        return true;
    }));
    return true;
}
#endif
