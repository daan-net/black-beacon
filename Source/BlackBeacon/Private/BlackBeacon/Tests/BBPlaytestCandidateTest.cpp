#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Engine/GameInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SpotLightComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputKeyEventArgs.h"
#include "HAL/PlatformTime.h"
#include "BlackBeacon/Core/BBPlayerCharacter.h"
#include "BlackBeacon/Core/BBPlayerController.h"
#include "BlackBeacon/Lighthouse/BBLighthouseController.h"
#include "BlackBeacon/Lighthouse/BBLighthouseBeamComponent.h"
#include "BlackBeacon/Lighthouse/BBBeamControlComponent.h"
#include "BlackBeacon/Lighthouse/BBBeamRevealComponent.h"
#include "BlackBeacon/Interaction/BBInteractionComponent.h"
#include "BlackBeacon/Power/BBGeneratorComponent.h"
#include "BlackBeacon/Objectives/BBObjectiveSystem.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBBPlaytestCandidateTest, "BlackBeacon.V051.PlaytestLoop",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FBBPlaytestCandidateTest::RunTest(const FString& Parameters)
{
    struct FState
    {
        int32 Stage=0, RoutePoint=0, EntryPass=0;
        double Start=FPlatformTime::Seconds(), At=Start;
        UWorld* World=nullptr;
        ABBlackBeaconPlayerController* PC=nullptr;
        ABBlackBeaconPlayerCharacter* Pawn=nullptr;
        ABBLighthouseController* Lighthouse=nullptr;
        UBBGeneratorComponent* Generator=nullptr;
        UBBBeamRevealComponent* Reveal=nullptr;
        UBBObjectiveSystem* Objectives=nullptr;
        FVector Target, StopPosition;
        bool bPaused=false, bMidScreenshot=false;
        float LastPitch=0;
    };
    const auto State=MakeShared<FState>();
    ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this,State]()
    {
        const double Now=FPlatformTime::Seconds();
        const auto Key=[&](FKey Key,EInputEvent Event,float Amount)
        {
            State->PC->InputKey(FInputKeyEventArgs(nullptr,INPUTDEVICEID_NONE,Key,Event,Amount,false,FPlatformTime::Cycles64()));
        };
        const auto Stage=[&](int32 Next)
        {
            State->Stage=Next;State->At=Now;
            AddInfo(FString::Printf(TEXT("V051 stage %d pawn %s"),Next,*State->Pawn->GetActorLocation().ToString()));
        };
        if (Now-State->Start>150)
        {
            AddError(FString::Printf(TEXT("V051 timeout stage %d pawn %s yaw %.1f"),State->Stage,
                State->Pawn ? *State->Pawn->GetActorLocation().ToString() : TEXT("none"),State->PC ? State->PC->GetControlRotation().Yaw : 0));
            if(State->PC) Key(EKeys::W,IE_Released,0);
            FScreenshotRequest::RequestScreenshot(TEXT("V051_Failure.png"),false,false);
            return true;
        }
        if(State->Stage==0)
        {
            if(Now-State->Start<4) return false;
            for(const auto& Context:GEngine->GetWorldContexts())
                if(Context.WorldType==EWorldType::Game && Context.World() && Context.World()->HasBegunPlay()) State->World=Context.World();
            if(!State->World) return false;
            State->PC=Cast<ABBlackBeaconPlayerController>(State->World->GetFirstPlayerController());
            State->Pawn=State->PC ? Cast<ABBlackBeaconPlayerCharacter>(State->PC->GetPawn()) : nullptr;
            for(TActorIterator<AActor> It(State->World);It;++It)
            {
                if(auto* LH=Cast<ABBLighthouseController>(*It)) State->Lighthouse=LH;
                if(auto* Gen=It->FindComponentByClass<UBBGeneratorComponent>()) State->Generator=Gen;
                if(It->ActorHasTag(TEXT("BB_Anomaly")))
                {
                    State->Reveal=It->FindComponentByClass<UBBBeamRevealComponent>();
                    for(auto* Mesh:TInlineComponentArray<UStaticMeshComponent*>(*It))
                        if(Mesh->GetName()==TEXT("RevealedRuinPart_00")) State->Target=Mesh->Bounds.Origin;
                }
            }
            if(!TestNotNull(TEXT("Playable pawn"),State->Pawn) || !TestNotNull(TEXT("Lighthouse"),State->Lighthouse)
                || !TestNotNull(TEXT("Generator"),State->Generator) || !TestNotNull(TEXT("Wreck"),State->Reveal)) return true;
            State->Objectives=State->World->GetGameInstance()->GetSubsystem<UBBObjectiveSystem>();
            AddInfo(FString::Printf(TEXT("Wreck hull search point: %s"),*State->Target.ToString()));
            FCollisionQueryParams Params;
            // Isolate the perimeter for these geometry probes. Actual walking below
            // exercises the complete collision scene, including step-up and rails.
            for(TActorIterator<AActor> It(State->World);It;++It)
                if(*It!=State->Lighthouse) Params.AddIgnoredActor(*It);
            const auto Blocked=[&](FVector A,FVector B)
            {
                FHitResult Hit;
                const bool bHit=State->World->SweepSingleByChannel(Hit,A,B,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(38,88),Params);
                if(bHit) AddInfo(FString::Printf(TEXT("Capsule sweep %s -> %s hit %s/%s at %s"),
                    *A.ToString(),*B.ToString(),*GetNameSafe(Hit.GetActor()),*GetNameSafe(Hit.GetComponent()),*Hit.ImpactPoint.ToString()));
                return bHit;
            };
            for(float Angle:{30.f,60.f,90.f,135.f,180.f,225.f,270.f,330.f})
            {
                const FVector D=FRotator(0,Angle,0).Vector();
                TestTrue(*FString::Printf(TEXT("Tower blocks entry at %.0f degrees"),Angle),Blocked(D*430+FVector(0,0,112),D*280+FVector(0,0,112)));
            }
            for(float Angle:{-3.f,0.f,3.f})
            {
                const FVector D=FRotator(0,Angle,0).Vector();
                TestFalse(*FString::Printf(TEXT("Actual doorway clear at %.0f degrees"),Angle),Blocked(D*425+FVector(0,0,150),D*280+FVector(0,0,150)));
            }
            // Include stair guards and landings: perimeter-only probes missed the
            // first guard's projecting corner even at the higher capsule position.
            FCollisionQueryParams EntryParams;
            EntryParams.AddIgnoredActor(State->Pawn);
            for (float Height : {112.0f, 150.0f})
            {
                for (float Angle : {-3.0f, 0.0f, 3.0f})
                {
                    const FVector Direction = FRotator(0, Angle, 0).Vector();
                    FHitResult Hit;
                    const bool bBlocked = State->World->SweepSingleByChannel(Hit,
                        Direction * 420 + FVector(0, 0, Height), Direction * 280 + FVector(0, 0, Height),
                        FQuat::Identity, ECC_Pawn, FCollisionShape::MakeCapsule(38, 88), EntryParams);
                    // A level sweep can hit a tread that CharacterMovement steps onto.
                    // Do not exempt guards, walls, or risers above the actual step limit.
                    const AActor* HitActor = Hit.GetActor();
                    const UPrimitiveComponent* HitComponent = Hit.GetComponent();
                    const bool bReachableTread = HitActor && HitComponent
                        && HitActor->ActorHasTag(TEXT("BB_StairStep"))
                        && State->Pawn->GetCharacterMovement()->CanStepUp(Hit)
                        && HitComponent->Bounds.GetBox().Max.Z <= Height - 88.0f
                            + State->Pawn->GetCharacterMovement()->MaxStepHeight;
                    TestTrue(*FString::Printf(TEXT("Full-scene entry permits walking at %.0f degrees, Z=%.0f (hit %s/%s)"),
                        Angle, Height, *GetNameSafe(HitActor), *GetNameSafe(HitComponent)), !bBlocked || bReachableTread);
                }
            }
            for(float Angle:{45.f,90.f,180.f,270.f})
            {
                const FVector D=FRotator(0,Angle,0).Vector();
                TestTrue(TEXT("Lantern wall prevents balcony shortcut"),Blocked(D*325+FVector(0,0,1660),D*200+FVector(0,0,1660)));
            }
            TestFalse(TEXT("Balcony service door clears standing capsule"),Blocked(FVector(325,0,1688),FVector(170,0,1688)));
            State->Pawn->SetActorLocation(FVector(419.424,-21.982,112));State->PC->SetControlRotation(FRotator(0,177,0));
            Key(EKeys::W,IE_Pressed,1);Stage(1);return false;
        }
        auto* Pawn=State->Pawn;auto* PC=State->PC;auto* Beam=State->Lighthouse->BeamComponent.Get();
        auto* Control=PC->FindComponentByClass<UBBBeamControlComponent>();
        if(State->Stage==1 && Now-State->At>.6)
        {
            Key(EKeys::W,IE_Released,0);
            AddInfo(FString::Printf(TEXT("Doorway approach %d: location=%s velocity=%s"),
                State->EntryPass, *Pawn->GetActorLocation().ToString(), *Pawn->GetVelocity().ToString()));
            TestTrue(*FString::Printf(TEXT("W enters the genuine doorway, approach %d"),State->EntryPass),Pawn->GetActorLocation().X<280);
            if(++State->EntryPass<3)
            {
                const float Angle=State->EntryPass==1 ? 0.0f : 3.0f;
                Pawn->SetActorLocation(FRotator(0,Angle,0).Vector()*420+FVector(0,0,112));
                Pawn->GetCharacterMovement()->StopMovementImmediately();
                PC->SetControlRotation(FRotator(0,180+Angle,0));Key(EKeys::W,IE_Pressed,1);Stage(1);return false;
            }
            TestTrue(TEXT("Entry objective advances"),State->Objectives->IsCompleted(TEXT("BB_OBJ_ENTER_LIGHTHOUSE")));
            Pawn->SetActorLocation(FVector(-200,-620,100));PC->SetControlRotation(FRotator(0,0,0));
            Stage(2);
        }
        else if(State->Stage==2 && Now-State->At>.6)
        {
            TestTrue(TEXT("Generator focuses through normal interaction trace"),Pawn->GetInteractionComponent()->GetFocusedActor()==State->Generator->GetOwner());
            Key(EKeys::E,IE_Pressed,1);Stage(3);
        }
        else if(State->Stage==3 && Now-State->At>.2)
        {
            Key(EKeys::E,IE_Released,0);
            if(State->Generator->IsProducing())
            {
                TestTrue(TEXT("Generator powers lighthouse"),State->Lighthouse->IsPowered());
                TestTrue(TEXT("Generator objective advances"),State->Objectives->IsCompleted(TEXT("BB_OBJ_START_GENERATOR")));
                Pawn->SetActorLocation(FVector(178,0,112));Pawn->GetCharacterMovement()->StopMovementImmediately();
                PC->SetControlRotation(FRotator(-8,-90,0));Stage(4);
            }
        }
        else if(State->Stage==4 && Now-State->At>.6)
        {
            Key(EKeys::W,IE_Pressed,1);Stage(5);
        }
        else if(State->Stage==5)
        {
            const float Feet=Pawn->GetActorLocation().Z-88;
            if(Feet>650 && !State->bPaused)
            {
                Key(EKeys::W,IE_Released,0);State->bPaused=true;Stage(6);
            }
            else if(Feet>1100 && !State->bMidScreenshot)
            {
                FScreenshotRequest::RequestScreenshot(TEXT("V051_StairClimb.png"),true,false);State->bMidScreenshot=true;
            }
            else if(Feet>=1570 && Pawn->GetActorLocation().Y<=0)
            {
                Key(EKeys::W,IE_Released,0);
                TestTrue(TEXT("Full climb with forward input, no waypoint or repeated mouse steering"),true);
                Stage(8);
            }
        }
        else if(State->Stage==6 && Now-State->At>.4)
        {
            State->StopPosition=Pawn->GetActorLocation();Stage(7);
        }
        else if(State->Stage==7 && Now-State->At>.8)
        {
            TestTrue(TEXT("Stair assistance stops when player releases W"),FVector::Dist(Pawn->GetActorLocation(),State->StopPosition)<3);
            Key(EKeys::W,IE_Pressed,1);Stage(5);
        }
        else if(State->Stage==8 && Now-State->At>.5)
        {
            TestTrue(TEXT("Full climb reaches lantern objective"),State->Objectives->IsCompleted(TEXT("BB_OBJ_CLIMB")));
            // Walk across the supported landing to the console. No teleport past the hatch.
            const FVector Route[]={FVector(155,-30,1660),FVector(325,-30,1660),FVector(155,-30,1660),
                FVector(140,-140,1660),FVector(-75,-140,1660),FVector(-75,-85,1660)};
            const FVector Delta=Route[State->RoutePoint]-Pawn->GetActorLocation();
            if(Delta.Size2D()>18) {Pawn->AddMovementInput(FVector(Delta.X,Delta.Y,0).GetSafeNormal(),1);return false;}
            if(State->RoutePoint==1)
            {
                PC->SetControlRotation((FVector(240,0,1690)-Pawn->GetFirstPersonCamera()->GetComponentLocation()).Rotation());
                FScreenshotRequest::RequestScreenshot(TEXT("V051_BalconyDoor.png"),true,false);
            }
            if(++State->RoutePoint<UE_ARRAY_COUNT(Route)) return false;
            TestTrue(TEXT("Player walks through service door and returns to lantern console"),true);
            PC->SetControlRotation((FVector(-150,-85,1660)-Pawn->GetFirstPersonCamera()->GetComponentLocation()).Rotation());
            Stage(9);
        }
        else if(State->Stage==9 && Now-State->At>.6)
        {
            FScreenshotRequest::RequestScreenshot(TEXT("V051_ConsoleAccess.png"),true,false);
            TestTrue(TEXT("Console focuses from reachable standing position"),Pawn->GetInteractionComponent()->GetFocusedActor()==State->Lighthouse);
            Key(EKeys::E,IE_Pressed,1);Stage(10);
        }
        else if(State->Stage==10 && Now-State->At>.25)
        {
            Key(EKeys::E,IE_Released,0);
            TestTrue(TEXT("Beam starts"),Beam->IsPowered());Stage(11);
        }
        else if(State->Stage==11 && Now-State->At>.5)
        {
            Key(EKeys::E,IE_Pressed,1);Stage(12);
        }
        else if(State->Stage==12 && Now-State->At>1.5)
        {
            Key(EKeys::E,IE_Released,0);
            TestTrue(TEXT("E acquires search control"),Control->IsActive());
            TestFalse(TEXT("Acquisition alone does not reveal wreck"),State->Reveal->WasFullyRevealed());
            TestTrue(TEXT("Search view faces exterior, not console"),PC->GetViewTarget()!=Pawn);
            State->LastPitch=Beam->GetCurrentPitchDegrees();Key(EKeys::MouseY,IE_Axis,-25);Stage(13);
        }
        else if(State->Stage==13 && Now-State->At>.3)
        {
            AddInfo(FString::Printf(TEXT("Mouse pitch: before %.3f, after %.3f"),State->LastPitch,Beam->GetCurrentPitchDegrees()));
            TestTrue(TEXT("Mouse Y changes beam pitch"),FMath::Abs(Beam->GetCurrentPitchDegrees()-State->LastPitch)>.1);
            TestTrue(TEXT("Operator view excludes the exterior shaft proxy"),PC->HiddenPrimitiveComponents.Num()>0);
            const auto Query=Beam->GetBeamQuery();
            const FVector Direction(Query.Direction.X,Query.Direction.Y,Query.Direction.Z);
            TestTrue(TEXT("Pitch-only input synchronizes spotlight and reveal query"),
                State->Lighthouse->FindComponentByClass<USpotLightComponent>()->GetForwardVector().Equals(Direction,.001));
            Stage(14);
        }
        else if(State->Stage==14)
        {
            // Feed mouse counts through Enhanced Input while searching; never force a reveal.
            const FRotator Aim=(State->Target-Beam->GetComponentLocation()).Rotation();
            const float Yaw=FMath::FindDeltaAngleDegrees(Beam->GetCurrentYawDegrees(),Aim.Yaw);
            const float Pitch=Aim.Pitch-Beam->GetCurrentPitchDegrees();
            if(FMath::Abs(Yaw)>0.15) Key(EKeys::MouseX,IE_Axis,FMath::Clamp(Yaw,-1.5f,1.5f)/.22f);
            if(FMath::Abs(Pitch)>.1) Key(EKeys::MouseY,IE_Axis,FMath::Clamp(Pitch,-1.f,1.f)/.22f);
            if(State->Reveal->WasFullyRevealed() && State->Reveal->GetVisibilityAmount()>.999)
            {
                TestTrue(TEXT("Manual two-axis search discovers wreck"),State->Reveal->WasFullyRevealed());
                TestTrue(TEXT("Discovery objective acknowledged"),State->Objectives->IsCompleted(TEXT("BB_OBJ_DISCOVER_ANOMALY")));
                FScreenshotRequest::RequestScreenshot(TEXT("V051_WreckDiscovery.png"),true,false);Stage(15);
            }
        }
        else if(State->Stage==15 && Now-State->At>1)
        {
            Key(EKeys::E,IE_Pressed,1);Stage(16);
        }
        else if(State->Stage==16 && Now-State->At>1.2)
        {
            Key(EKeys::E,IE_Released,0);
            TestFalse(TEXT("E releases without looking back at console"),Control->IsActive());
            TestTrue(TEXT("Pawn view and movement restored"),PC->GetViewTarget()==Pawn && !PC->IsMoveInputIgnored());
            TestTrue(TEXT("Wreck fades after releasing search"),State->Reveal->GetVisibilityAmount()<.1);
            Stage(18);
        }
        else if(State->Stage==18 && Now-State->At>.3)
        {
            Key(EKeys::E,IE_Pressed,1);Stage(17);
        }
        else if(State->Stage==17 && Now-State->At>.5)
        {
            Key(EKeys::E,IE_Released,0);
            TestTrue(TEXT("Console reacquires after release"),Control->IsActive());
            TestTrue(TEXT("Discovery objective persists across release and reacquire"),State->Objectives->IsCompleted(TEXT("BB_OBJ_DISCOVER_ANOMALY")));
            Control->Release();
            AddInfo(TEXT("V051 essential playtest loop complete"));return true;
        }
        return false;
    }));
    return true;
}
#endif
