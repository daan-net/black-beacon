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
#include "Components/VolumetricCloudComponent.h"
#include "HighResScreenshot.h"
#include "UnrealClient.h"
#include "AudioMixerBlueprintLibrary.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "BlackBeacon/Weather/BBWeatherController.h"
#include "BlackBeacon/Weather/BBStormPresentationComponent.h"
#include "BlackBeacon/Weather/BBStormAudioComponent.h"
#include "BlackBeacon/Power/BBGeneratorComponent.h"
#include "BlackBeacon/Lighthouse/BBLighthouseController.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBBStormWorldTest,"BlackBeacon.StormWorld.Identity",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FBBStormWorldTest::RunTest(const FString& Parameters)
{
    struct FState
    {
        UWorld* World=nullptr;
        TWeakObjectPtr<ABBWeatherController> Weather;
        TWeakObjectPtr<ABBLighthouseController> Lighthouse;
        TWeakObjectPtr<ACameraActor> Camera;
        TWeakObjectPtr<APawn> Pawn;
        FVector PawnLocation;
        FRotator PawnRotation;
        FVector Target=FVector(-5200,4200,500);
        double Started=FPlatformTime::Seconds();
        double StageAt=Started;
        int32 Index=-1;
        int32 ThunderBefore=0;
        bool bRequested=false;
        bool bFlashTriggered=false;
    };
    TSharedRef<FState> State=MakeShared<FState>();
    ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this,State]()
    {
        const double Now=FPlatformTime::Seconds();
        if (Now-State->Started>100) { AddError(TEXT("Storm identity capture timed out"));return true; }
        if (!State->World)
        {
            for (const FWorldContext& C:GEngine->GetWorldContexts())
                if (C.WorldType==EWorldType::Game && C.World() && C.World()->HasBegunPlay()) State->World=C.World();
            if (!State->World) return false;
            UWorld* W=State->World;
            for (TActorIterator<ABBWeatherController> I(W);I;++I) State->Weather=*I;
            for (TActorIterator<ABBLighthouseController> I(W);I;++I) State->Lighthouse=*I;
            if (!State->Weather.IsValid() || !State->Lighthouse.IsValid()) { AddError(TEXT("Storm/lighthouse actor absent"));return true; }
            State->Weather->SetWeather(EBBWeatherPhase::Storm,0);
            APlayerController* PC=W->GetFirstPlayerController();
            State->Pawn=PC->GetPawn();State->PawnLocation=State->Pawn->GetActorLocation();State->PawnRotation=PC->GetControlRotation();
            for (TActorIterator<AActor> I(W);I;++I)
            {
                if (UBBGeneratorComponent* G=I->FindComponentByClass<UBBGeneratorComponent>()) G->Start();
                if (I->ActorHasTag(TEXT("BB_Anomaly")))
                {
                    for (UStaticMeshComponent* M:TInlineComponentArray<UStaticMeshComponent*>(*I))
                        if (M->ComponentHasTag(TEXT("BB_BeamRevealPart"))) { State->Target=M->Bounds.Origin;break; }
                }
            }
            for (TActorIterator<AActor> I(W); I; ++I)
            {
                if (!I->ActorHasTag(TEXT("BB_Ground"))) continue;
                UStaticMeshComponent* Ground=I->FindComponentByClass<UStaticMeshComponent>();
                if (!Ground) continue;
                for (const FVector Point : {FVector(-4500,-600,0), FVector(-3200,-350,0), FVector(-1600,-100,0)})
                {
                    FHitResult Hit;
                    const bool bHit=Ground->LineTraceComponent(Hit,Point+FVector(0,0,300),Point-FVector(0,0,900),FCollisionQueryParams(NAME_None,true));
                    TestTrue(TEXT("Coastal terrain preserves the shore-to-lighthouse route"),bHit && FMath::Abs(Hit.ImpactPoint.Z)<5.0f);
                }
            }
            State->StageAt=Now;
            UAudioMixerBlueprintLibrary::StartRecordingOutput(W,50.0f);
            GEngine->Exec(W,TEXT("csvprofile start"));
            return false;
        }
        ABBWeatherController* Weather=State->Weather.Get();
        UBBStormPresentationComponent* Storm=Weather->StormPresentation;
        if (State->Index<0)
        {
            if (Now-State->StageAt<5.0) return false;
            TestTrue(TEXT("Native cloud/ocean/ambient layers initialized"),Storm && Storm->IsReady());
            TestTrue(TEXT("Static panorama is hidden"),!Weather->SkyCloudDome->IsVisible());
            TestTrue(TEXT("Storm wind is diagonal and strong"),Weather->GetWindVelocity().X>500 && Weather->GetWindVelocity().Y>100);
            TestTrue(TEXT("Volumetric cloud material is assigned"),Weather->StormClouds->GetMaterial()!=nullptr);
            if (!Storm || !Storm->IsReady()) return true;
            State->Lighthouse->RequestBeamStart();
            State->Lighthouse->BeamComponent->SetRotationMode(EBBBeamRotationMode::Manual);
            const FRotator Aim=(State->Target-State->Lighthouse->BeamComponent->GetComponentLocation()).Rotation();
            State->Lighthouse->BeamComponent->SetManualYawTarget(Aim.Yaw);
            State->Lighthouse->BeamComponent->SetManualPitchDegrees(Aim.Pitch);
            State->Camera=State->World->SpawnActor<ACameraActor>();
            State->World->GetFirstPlayerController()->SetViewTarget(State->Camera.Get());
            State->Index=0; State->StageAt=Now;
        }
        const FVector Positions[]={
            {-4200,-2900,700}, {-3900,-1550,420}, {-3400,-650,240},
            {-2800,-200,180}, {-2700,-2200,360}, {-2600,-3000,900},
            {-1800,-1400,1900}, {130,0,210}, {-3200,-2200,500}, {-1500,1200,1700},
            {-4200,-2900,700}
        };
        const FVector Targets[]={
            {0,0,1550}, {-4500,-4600,-200}, {-3400,-1300,0},
            {0,0,550}, {-1700,-1200,30}, {0,0,1850},
            {-4200,3300,950}, {0,90,450}, {0,0,1200}, State->Target,
            {0,0,1550}
        };
        const TCHAR* Names[]={
            TEXT("BlackBeacon_Storm_A_LighthouseSky.png"),TEXT("BlackBeacon_Storm_B_RoughOcean.png"),
            TEXT("BlackBeacon_Storm_C_WetRocks.png"),TEXT("BlackBeacon_Storm_D_Rain.png"),
            TEXT("BlackBeacon_Storm_E_CoastalMist.png"),TEXT("BlackBeacon_Storm_F_Lightning.png"),
            TEXT("BlackBeacon_Storm_G_BeaconFog.png"),TEXT("BlackBeacon_Storm_H_Interior.png"),
            TEXT("BlackBeacon_Storm_I_Night.png"),TEXT("BlackBeacon_Storm_J_WreckReveal.png"),
            TEXT("BlackBeacon_Storm_A2_CloudMovement.png")
        };
        if (!State->bRequested)
        {
            State->Camera->SetActorLocation(Positions[State->Index]);
            State->Camera->SetActorRotation((Targets[State->Index]-Positions[State->Index]).Rotation());
            State->Camera->GetCameraComponent()->SetFieldOfView(State->Index==9 ? 55 : 78);
            if (Now-State->StageAt<2.0) return false;
            if (State->Index==5 && !State->bFlashTriggered)
            {
                State->ThunderBefore=Storm->GetStormAudio()->GetThunderCount();
                Storm->TriggerLightning(1029);State->bFlashTriggered=true;State->StageAt=Now-1.85;
                return false;
            }
            if (State->Index==5)
            {
                TestTrue(TEXT("Lightning has a visible flash envelope"),Storm->GetLightningIntensity()>0.1f);
                TestEqual(TEXT("Thunder has not played at the flash"),Storm->GetStormAudio()->GetThunderCount(),State->ThunderBefore);
            }
            if (State->Index==7)
            {
                TestTrue(TEXT("Interior shelter muffles ambient layers"),Storm->GetStormAudio()->GetShelterMix()>0.5f);
                TestTrue(TEXT("Thunder arrives later than its flash"),Storm->GetStormAudio()->GetThunderCount()>State->ThunderBefore);
            }
            FScreenshotRequest::RequestScreenshot(Names[State->Index],false,false);
            State->bRequested=true;State->StageAt=Now;return false;
        }
        if (Now-State->StageAt<0.3) return false;
        ++State->Index;State->bRequested=false;State->StageAt=Now;
        if (State->Index<UE_ARRAY_COUNT(Positions)) return false;
        const FString Output=FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("StormReview"));
        IFileManager::Get().MakeDirectory(*Output,true);
        UAudioMixerBlueprintLibrary::StopRecordingOutput(State->World,EAudioRecordingExportType::WavFile,
            TEXT("BlackBeacon_Storm_RuntimeMix"),Output);
        APlayerController* PC=State->World->GetFirstPlayerController();
        PC->SetViewTarget(State->Pawn.Get());PC->SetControlRotation(State->PawnRotation);
        GEngine->Exec(State->World,TEXT("csvprofile stop"));
        State->Camera->Destroy();
        TestTrue(TEXT("Storm beacon remains powered through captures"),State->Lighthouse->BeamComponent->IsPowered());
        return true;
    }));
    return true;
}
#endif
