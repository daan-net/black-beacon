#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "UnrealClient.h"
#include "BlackBeacon/Weather/BBWeatherController.h"
#include "BlackBeacon/Power/BBGeneratorComponent.h"
#include "BlackBeacon/Lighthouse/BBLighthouseController.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBBVisualRebuildTest,"BlackBeacon.V04.ArchitectureReview",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FBBVisualRebuildTest::RunTest(const FString& Parameters)
{
    struct FState
    {
        UWorld* World=nullptr;
        TWeakObjectPtr<ABBLighthouseController> Lighthouse;
        TWeakObjectPtr<ACameraActor> Camera;
        TWeakObjectPtr<AActor> PreviousView;
        FVector Target=FVector(-5200,4200,500);
        double Start=FPlatformTime::Seconds();
        double Stage=Start;
        int32 Index=-1;
        bool Requested=false;
    };
    TSharedRef<FState> State=MakeShared<FState>();
    ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this,State]()
    {
        const double Now=FPlatformTime::Seconds();
        if (Now-State->Start>150) { AddError(TEXT("V0.4 review timed out"));return true; }
        if (!State->World)
        {
            for (const FWorldContext& C:GEngine->GetWorldContexts())
                if (C.WorldType==EWorldType::Game && C.World() && C.World()->HasBegunPlay()) State->World=C.World();
            if (!State->World) return false;
            int32 Directions=0;
            for (TActorIterator<AActor> I(State->World);I;++I)
            {
                Directions+=TInlineComponentArray<UDirectionalLightComponent*>(*I).Num();
                if (ABBWeatherController* Weather=Cast<ABBWeatherController>(*I)) Weather->SetWeather(EBBWeatherPhase::Storm,0);
                if (ABBLighthouseController* L=Cast<ABBLighthouseController>(*I)) State->Lighthouse=L;
                if (UBBGeneratorComponent* G=I->FindComponentByClass<UBBGeneratorComponent>()) G->Start();
                if (I->ActorHasTag(TEXT("BB_Anomaly")))
                    for (UStaticMeshComponent* M:TInlineComponentArray<UStaticMeshComponent*>(*I))
                        if (M->ComponentHasTag(TEXT("BB_BeamRevealPart"))) { State->Target=M->Bounds.Origin;break; }
            }
            TestEqual(TEXT("Exactly one directional light serves the storm"),Directions,1);
            if (!State->Lighthouse.IsValid()) { AddError(TEXT("No lighthouse"));return true; }
            State->PreviousView=State->World->GetFirstPlayerController()->GetViewTarget();
            State->Stage=Now;return false;
        }
        if (State->Index<0)
        {
            if (Now-State->Stage<6) return false;
            State->Lighthouse->RequestBeamStart();
            const FRotator Aim=(State->Target-State->Lighthouse->BeamComponent->GetComponentLocation()).Rotation();
            State->Lighthouse->BeamComponent->SetManualYawTarget(Aim.Yaw);
            State->Lighthouse->BeamComponent->SetManualPitchDegrees(Aim.Pitch);
            UStaticMeshComponent* Rotor=nullptr;
            UStaticMeshComponent* Deck=nullptr;
            for (UStaticMeshComponent* M:TInlineComponentArray<UStaticMeshComponent*>(State->Lighthouse.Get()))
            {
                if (M->GetName()==TEXT("HeroFresnelRotor")) Rotor=M;
                if (M->GetName()==TEXT("HeroGalleryDeck")) Deck=M;
            }
            TestTrue(TEXT("Fresnel mechanism follows existing beam pivot without collision"),Rotor && Rotor->GetStaticMesh()
                && Rotor->GetAttachParent()->GetName()==TEXT("BeamVisualPivot") && Rotor->GetCollisionEnabled()==ECollisionEnabled::NoCollision);
            TestTrue(TEXT("Gallery floor has supporting collision"),Deck && Deck->GetCollisionEnabled()==ECollisionEnabled::QueryAndPhysics);
            if (Deck)
            {
                for (const FVector P:{FVector(-180,0,1570),FVector(310,0,1570),FVector(0,-300,1570)})
                {
                    FHitResult Hit;
                    TestTrue(TEXT("Gallery floor agrees with its visible surface"),Deck->LineTraceComponent(Hit,P+FVector(0,0,60),P-FVector(0,0,60),FCollisionQueryParams(NAME_None,true)));
                }
                FHitResult Hit;
                TestFalse(TEXT("Last stair flight retains overhead hatch clearance"),Deck->LineTraceComponent(Hit,FVector(110,85,1590),FVector(110,85,1500),FCollisionQueryParams(NAME_None,true)));
            }
            State->Camera=State->World->SpawnActor<ACameraActor>();
            State->World->GetFirstPlayerController()->SetViewTarget(State->Camera.Get());
            State->Index=0;State->Stage=Now;
        }
        const FVector Positions[]={
            {-4200,-2900,700},{-1900,-2300,1050},{850,-750,190},{-650,-1120,230},
            {-110,-690,145},{210,15,155},{158,12,665},{130,12,1190},
            {175,-95,1740},{155,-150,1940},{-2300,-1900,800},{-2300,-1900,800},
            {-4200,-2900,700},{-3400,-650,240},{-2200,1000,1650}
        };
        const FVector Targets[]={
            {0,0,1550},{0,0,1170},{170,0,180},{0,-620,160},
            {60,-560,140},{80,-130,250},{40,-125,720},{15,-125,1230},
            {0,45,1960},{0,0,1960},{0,0,1530},{0,0,1530},
            {-500,800,4600},{-3400,-1600,-100},State->Target
        };
        const TCHAR* Names[]={
            TEXT("A_FullLighthouse"),TEXT("B_HeroThreeQuarter"),TEXT("C_Base"),TEXT("D_AnnexExterior"),TEXT("E_AnnexInterior"),
            TEXT("F_LowerStairs"),TEXT("G_MidTransition"),TEXT("H_UpperStairs"),TEXT("I_LanternRoom"),TEXT("J_Fresnel"),
            TEXT("K_BeaconOff"),TEXT("L_BeaconOn"),TEXT("M_StormSky"),TEXT("N_CoastOcean"),TEXT("O_WreckReveal")
        };
        if (!State->Requested)
        {
            State->Camera->SetActorLocation(Positions[State->Index]);
            State->Camera->SetActorRotation((Targets[State->Index]-Positions[State->Index]).Rotation());
            State->Camera->GetCameraComponent()->SetFieldOfView(State->Index==9 ? 65 : 78);
            // Exercise the actual public state restore path; it controls lens, glow and beam together.
            if (State->Index==10 && State->Lighthouse->HasStartedBeam())
                State->Lighthouse->RestoreBeamState(false,true,EBBBeamRotationMode::Off,State->Lighthouse->BeamComponent->GetCurrentYawDegrees(),State->Lighthouse->BeamComponent->GetCurrentPitchDegrees());
            if (State->Index==11 && !State->Lighthouse->HasStartedBeam())
            {
                State->Lighthouse->RequestBeamStart();
                const FRotator Aim=(State->Target-State->Lighthouse->BeamComponent->GetComponentLocation()).Rotation();
                State->Lighthouse->BeamComponent->SetManualYawTarget(Aim.Yaw);
                State->Lighthouse->BeamComponent->SetManualPitchDegrees(Aim.Pitch);
            }
            if (Now-State->Stage<3.5) return false;
            FScreenshotRequest::RequestScreenshot(FString::Printf(TEXT("BlackBeacon_V04_%s.png"),Names[State->Index]),false,false);
            State->Requested=true;State->Stage=Now;return false;
        }
        if (Now-State->Stage<.4) return false;
        ++State->Index;State->Requested=false;State->Stage=Now;
        if (State->Index<UE_ARRAY_COUNT(Positions)) return false;
        State->World->GetFirstPlayerController()->SetViewTarget(State->PreviousView.Get());
        State->Camera->Destroy();
        TestTrue(TEXT("Beam remains powered after architectural review"),State->Lighthouse->BeamComponent->IsPowered());
        return true;
    }));
    return true;
}
#endif
