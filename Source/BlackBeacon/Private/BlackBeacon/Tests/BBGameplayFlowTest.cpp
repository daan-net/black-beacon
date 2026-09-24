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
#include "Components/PointLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInstanceDynamic.h"

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

    FVector GetRevealPartCenter(AActor* Actor)
    {
        if (!Actor)
        {
            return FVector::ZeroVector;
        }
        FVector Center = FVector::ZeroVector;
        int32 PartCount = 0;
        for (const UStaticMeshComponent* Mesh : TInlineComponentArray<UStaticMeshComponent*>(Actor))
        {
            if (Mesh && Mesh->ComponentHasTag(TEXT("BB_BeamRevealPart")))
            {
                Center += Mesh->Bounds.Origin;
                ++PartCount;
            }
        }
        return PartCount > 0 ? Center / static_cast<float>(PartCount) : Actor->GetActorLocation();
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
        TWeakObjectPtr<USceneComponent> GeneratorFlywheelPivot;
        TWeakObjectPtr<ABBLighthouseController> Lighthouse;
        TWeakObjectPtr<UInstancedStaticMeshComponent> LanternFresnelBands;
        TArray<TWeakObjectPtr<UStaticMeshComponent>> LanternGlazingPanels;
        TWeakObjectPtr<AActor> Anomaly;
        TWeakObjectPtr<UBBBeamRevealComponent> AnomalyReveal;
        TWeakObjectPtr<APawn> Pawn;
        TWeakObjectPtr<UBBInteractionComponent> Interaction;
        TWeakObjectPtr<ABBWeatherController> Weather;
        TWeakObjectPtr<UMaterialInstanceDynamic> BeamShaftMaterial;
        TWeakObjectPtr<ACameraActor> GeneratorCaptureCamera;
        TWeakObjectPtr<ACameraActor> HeroCaptureCamera;
        FVector RainFieldAnchor = FVector::ZeroVector;
        TWeakObjectPtr<ACameraActor> CaptureCamera;
        int32 CaptureIndex = 0;
        FVector AirCameraPosition = FVector::ZeroVector;
        FVector AirCameraTarget = FVector::ZeroVector;
        float BeamScatteringBeforeDiagnostic = 0.0f;
        float BeamOpacityBeforeDiagnostic = 0.0f;
        bool bOpeningCaptured = false;
        bool bHeroCapturesComplete = false;
        bool bHeroCaptureRequested = false;
        int32 HeroCaptureIndex = 0;
        bool bGeneratorCameraReady = false;
        bool bGeneratorCaptured = false;
        bool bFlywheelMotionChecked = false;
        bool bFlywheelCoastChecked = false;
        bool bRevealFadeChecked = false;
        bool bBeamScatteringDiagnosticCaptured = false;
        int32 RevealFadePhase = 0;
        double RevealCaptureStartedAt = 0.0;
        int32 TaggedRevealPartCount = 0;
        float GeneratorFlywheelStartRoll = 0.0f;
        float GeneratorFlywheelStopRoll = 0.0f;
        float RevealVisibilityBeforePowerLoss = 0.0f;
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
                FScreenshotRequest::RequestScreenshot(TEXT("BlackBeacon_M01_A_Exterior.png"), false, false);
                State->bOpeningCaptured = true;
                State->StageAt = Now;
                return false;
            }
            if (State->bOpeningCaptured && Now - State->StageAt < 0.3)
            {
                return false;
            }
            if (FApp::CanEverRender() && !State->bHeroCapturesComplete)
            {
                const FVector CameraPositions[] = {
                    FVector(6500.0f, -8000.0f, 1750.0f),
                    FVector(-2600.0f, -4300.0f, 420.0f),
                    FVector(0.0f, -170.0f, 650.0f),
                    FVector(0.0f, -240.0f, 1980.0f)
                };
                const FVector CameraTargets[] = {
                    FVector(0.0f, -220.0f, 1000.0f),
                    FVector(-165.0f, -620.0f, 150.0f),
                    FVector(190.0f, -60.0f, 790.0f),
                    FVector(0.0f, 0.0f, 1980.0f)
                };
                const float CameraFov[] = {55.0f, 65.0f, 80.0f, 65.0f};
                const TCHAR* CaptureNames[] = {
                    TEXT("BlackBeacon_Hero_A_ExteriorThreeQuarter.png"),
                    TEXT("BlackBeacon_Hero_B_AnnexEntrance.png"),
                    TEXT("BlackBeacon_Hero_C_Stairwell.png"),
                    TEXT("BlackBeacon_Hero_D_LanternRoom.png")
                };
                if (!State->HeroCaptureCamera.IsValid())
                {
                    ACameraActor* Camera = World->SpawnActor<ACameraActor>();
                    if (!Camera)
                    {
                        AddError(TEXT("Could not create a hero lighthouse capture camera"));
                        return true;
                    }
                    State->HeroCaptureCamera = Camera;
                    Camera->GetCameraComponent()->SetFieldOfView(CameraFov[0]);
                    Camera->SetActorLocation(CameraPositions[0]);
                    Camera->SetActorRotation((CameraTargets[0] - CameraPositions[0]).Rotation());
                    World->GetFirstPlayerController()->SetViewTargetWithBlend(Camera, 0.0f);
                    State->StageAt = Now;
                    return false;
                }
                if (!State->bHeroCaptureRequested)
                {
                    if (Now - State->StageAt < 0.8)
                    {
                        return false;
                    }
                    FScreenshotRequest::RequestScreenshot(CaptureNames[State->HeroCaptureIndex], false, false);
                    State->bHeroCaptureRequested = true;
                    State->StageAt = Now;
                    return false;
                }
                if (Now - State->StageAt < 0.05)
                {
                    return false;
                }
                State->bHeroCaptureRequested = false;
                ++State->HeroCaptureIndex;
                if (State->HeroCaptureIndex < UE_ARRAY_COUNT(CameraPositions))
                {
                    ACameraActor* Camera = State->HeroCaptureCamera.Get();
                    Camera->SetActorLocation(CameraPositions[State->HeroCaptureIndex]);
                    Camera->SetActorRotation((CameraTargets[State->HeroCaptureIndex]
                        - CameraPositions[State->HeroCaptureIndex]).Rotation());
                    Camera->GetCameraComponent()->SetFieldOfView(CameraFov[State->HeroCaptureIndex]);
                    State->StageAt = Now;
                    return false;
                }
                World->GetFirstPlayerController()->SetViewTargetWithBlend(
                    World->GetFirstPlayerController()->GetPawn(), 0.0f);
                State->HeroCaptureCamera->Destroy();
                State->HeroCaptureCamera.Reset();
                State->bHeroCapturesComplete = true;
                State->StageAt = Now;
                return false;
            }
            State->World = World;
            State->Pawn = World->GetFirstPlayerController() ? World->GetFirstPlayerController()->GetPawn() : nullptr;
            TestNotNull(TEXT("Player pawn"), State->Pawn.Get());
            const APlayerController* PlayerController = World->GetFirstPlayerController();
            if (State->Pawn.IsValid() && PlayerController)
            {
                const float LighthouseYaw = (FVector::ZeroVector - State->Pawn->GetActorLocation()).Rotation().Yaw;
                TestTrue(TEXT("Shore opening faces the lighthouse horizontally"),
                    FMath::Abs(FMath::FindDeltaAngleDegrees(PlayerController->GetControlRotation().Yaw, LighthouseYaw)) < 1.0f);
                TestTrue(TEXT("Shore opening tilts up enough to frame the lantern"),
                    PlayerController->GetControlRotation().Pitch >= 4.0f);
            }
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
                    TestNotNull(TEXT("Instanced coast rocks use the engine sample boulder mesh"), RockField->GetStaticMesh().Get());
                    TestEqual(TEXT("Coastal boulders block the player"),
                        RockField->GetCollisionResponseToChannel(ECC_Pawn), ECR_Block);
                    TestTrue(TEXT("Coastal boulders have collision enabled"),
                        RockField->GetCollisionEnabled() == ECollisionEnabled::QueryAndPhysics);
                    TestTrue(TEXT("Coastal rock field uses clustered lobe shapes"),
                        RockField->GetInstanceCount() > 150);
                    if (RockField->GetStaticMesh())
                    {
                        TestEqual(TEXT("Coastal rock field uses the intended boulder mesh"),
                            RockField->GetStaticMesh()->GetName(), FString(TEXT("PCG_Boulder_02")));
                    }
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
                UStaticMeshComponent* AnnexVisual = nullptr;
                for (UStaticMeshComponent* const Mesh : CoastMeshes)
                {
                    if (Mesh && Mesh->GetName() == TEXT("AnnexHeroDetails"))
                    {
                        AnnexVisual = Mesh;
                        break;
                    }
                }
                TestTrue(TEXT("Generator annex receives a non-colliding hero detail shell"),
                    AnnexVisual && AnnexVisual->GetStaticMesh()
                    && AnnexVisual->GetCollisionEnabled() == ECollisionEnabled::NoCollision);
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
            TestTrue(TEXT("Functional generator annex sits beside the lighthouse approach"),
                GeneratorActor->GetActorLocation().Equals(FVector(0.0f, -620.0f, 90.0f), 1.0f));
            UTexture2D* const LighthousePaint = LoadObject<UTexture2D>(nullptr,
                TEXT("/Game/BlackBeacon/Textures/T_LighthousePaintAlbedo.T_LighthousePaintAlbedo"));
            UMaterialInstanceDynamic* const TowerMaterial = State->Lighthouse->TowerExteriorSkin
                ? Cast<UMaterialInstanceDynamic>(State->Lighthouse->TowerExteriorSkin->GetMaterial(0)) : nullptr;
            TestNotNull(TEXT("Lighthouse weathered paint texture loads"), LighthousePaint);
            TestTrue(TEXT("Lighthouse exterior uses the weathered paint texture"),
                TowerMaterial && LighthousePaint
                && TowerMaterial->K2_GetTextureParameterValue(TEXT("RockAlbedo")) == LighthousePaint);
            TestTrue(TEXT("Gameplay lighthouse tower shell uses the tapered hero mesh"),
                State->Lighthouse->TowerExteriorSkin
                && State->Lighthouse->TowerExteriorSkin->GetStaticMesh()
                && State->Lighthouse->TowerExteriorSkin->GetStaticMesh()->GetName() == TEXT("SM_BB_LH_TowerShell"));
            TestTrue(TEXT("Hero tower shell remains non-colliding"),
                State->Lighthouse->TowerExteriorSkin
                && State->Lighthouse->TowerExteriorSkin->GetCollisionEnabled() == ECollisionEnabled::NoCollision);
            TestTrue(TEXT("Beacon optical origin remains at its gameplay transform"),
                State->Lighthouse->BeamComponent
                && State->Lighthouse->BeamComponent->GetComponentLocation().Equals(
                    State->Lighthouse->GetActorLocation(), 0.1f));
            const TCHAR* const HeroComponentNames[] = {
                TEXT("HeroGallery"), TEXT("HeroLanternRoom"), TEXT("HeroRockPlinth")
            };
            for (const TCHAR* HeroName : HeroComponentNames)
            {
                UStaticMeshComponent* HeroPart = nullptr;
                for (UStaticMeshComponent* const Mesh : TInlineComponentArray<UStaticMeshComponent*>(State->Lighthouse.Get()))
                {
                    if (Mesh && Mesh->GetName() == HeroName)
                    {
                        HeroPart = Mesh;
                        break;
                    }
                }
                TestTrue(FString::Printf(TEXT("Hero visual %s is loaded without collision"), HeroName),
                    HeroPart && HeroPart->GetStaticMesh()
                    && HeroPart->GetCollisionEnabled() == ECollisionEnabled::NoCollision);
            }
            AActor* TowerWall = FindTaggedActor(World, TEXT("BB_TowerWall"));
            UStaticMeshComponent* TowerWallMesh = TowerWall ? TowerWall->FindComponentByClass<UStaticMeshComponent>() : nullptr;
            UPointLightComponent* StairFill = TowerWall ? TowerWall->FindComponentByClass<UPointLightComponent>() : nullptr;
            TestTrue(TEXT("Obsolete blockout wall is hidden while its stair fill light remains active"),
                TowerWallMesh && !TowerWallMesh->IsVisible() && StairFill && StairFill->IsVisible());
            AActor* StairTread = FindTaggedActor(World, TEXT("BB_StairStep"));
            UStaticMeshComponent* StairTreadMesh = StairTread ? StairTread->FindComponentByClass<UStaticMeshComponent>() : nullptr;
            TestTrue(TEXT("Existing stair tread collision is retained under the iron material"),
                StairTreadMesh && StairTreadMesh->GetCollisionEnabled() == ECollisionEnabled::QueryAndPhysics
                && StairTreadMesh->GetMaterial(0)
                && StairTreadMesh->GetMaterial(0)->GetName() == TEXT("M_LH_DarkIron"));
            int32 StairCountByFloor[3] = {0, 0, 0};
            bool bStairsFitTaperedTower = true;
            for (TActorIterator<AActor> It(World); It; ++It)
            {
                if (!It->ActorHasTag(TEXT("BB_StairStep")))
                {
                    continue;
                }
                const FVector StepLocation = It->GetActorLocation();
                const int32 Floor = FMath::Clamp(FMath::FloorToInt(StepLocation.Z / 520.0f), 0, 2);
                ++StairCountByFloor[Floor];
				const float ExpectedRadius = Floor == 0 ? 190.0f : (Floor == 1 ? 155.0f : 130.0f);
                bStairsFitTaperedTower &= FMath::IsNearlyEqual(
                    FVector2D(StepLocation).Size(), ExpectedRadius, 0.5f);
            }
            TestTrue(TEXT("Saved stair actors are dressed to fit the hero tower taper"), bStairsFitTaperedTower);
            TestEqual(TEXT("Lower stair flight keeps all 28 treads"), StairCountByFloor[0], 28);
            TestEqual(TEXT("Middle stair flight keeps all 28 treads"), StairCountByFloor[1], 28);
            TestEqual(TEXT("Upper stair flight keeps all 28 treads"), StairCountByFloor[2], 28);
            TArray<UStaticMeshComponent*> GeneratorMeshes;
            GeneratorActor->GetComponents<UStaticMeshComponent>(GeneratorMeshes);
            int32 NonBlockingGeneratorDetails = 0;
            bool bHasFlywheel = false;
            bool bKeepsHiddenInteractionCollider = false;
            for (const UStaticMeshComponent* Mesh : GeneratorMeshes)
            {
                if (Mesh && Mesh->GetName() == TEXT("Mesh"))
                {
                    bKeepsHiddenInteractionCollider = !Mesh->IsVisible()
                        && Mesh->GetCollisionEnabled() != ECollisionEnabled::NoCollision;
                }
                if (Mesh && Mesh->GetName().StartsWith(TEXT("Generator")))
                {
                    bHasFlywheel |= Mesh->GetName() == TEXT("GeneratorFlywheel");
                    NonBlockingGeneratorDetails += Mesh->GetCollisionEnabled() == ECollisionEnabled::NoCollision ? 1 : 0;
                }
            }
            TestTrue(TEXT("Generator has a visible flywheel assembly"), bHasFlywheel);
            for (USceneComponent* Component : TInlineComponentArray<USceneComponent*>(GeneratorActor))
            {
                if (Component && Component->GetName() == TEXT("GeneratorFlywheelPivot"))
                {
                    State->GeneratorFlywheelPivot = Component;
                    break;
                }
            }
            TestNotNull(TEXT("Generator flywheel has a motorized pivot"), State->GeneratorFlywheelPivot.Get());
            TestTrue(TEXT("Generator retains its invisible interaction collider"), bKeepsHiddenInteractionCollider);
            TestTrue(TEXT("Generator machinery details stay nonblocking"), NonBlockingGeneratorDetails >= 7);
            AActor* Shed = FindTaggedActor(World, TEXT("BB_GeneratorShed"));
            UStaticMeshComponent* ShedBlockout = Shed ? Shed->FindComponentByClass<UStaticMeshComponent>() : nullptr;
            TestNotNull(TEXT("Generator annex shell source"), ShedBlockout);
            TestTrue(TEXT("Solid shed blockout is hidden behind the traversable shell"),
                ShedBlockout && !ShedBlockout->IsVisible());
            UPointLightComponent* ShedLight = nullptr;
            TActorIterator<ABBCoastalEnvironment> EnvironmentIt(World);
            if (EnvironmentIt)
            {
                ShedLight = EnvironmentIt->FindComponentByClass<UPointLightComponent>();
            }
            TestTrue(TEXT("Generator annex has a warm working light"), ShedLight && ShedLight->Intensity > 0.0f);
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
            State->AnomalyReveal = State->Anomaly->FindComponentByClass<UBBBeamRevealComponent>();
            TestNotNull(TEXT("Reveal component"), State->AnomalyReveal.Get());
            TestFalse(TEXT("The first anomaly is a transient beam reveal"),
                State->AnomalyReveal.IsValid() && State->AnomalyReveal->bPersistent);
            TArray<UStaticMeshComponent*> AnomalyMeshes;
            State->Anomaly->GetComponents<UStaticMeshComponent>(AnomalyMeshes);
            TestTrue(TEXT("Anomaly has a modular ruin silhouette"), AnomalyMeshes.Num() >= 8);
            int32 TaggedRevealParts = 0;
            for (const UStaticMeshComponent* Mesh : AnomalyMeshes)
            {
                TaggedRevealParts += Mesh && Mesh->ComponentHasTag(TEXT("BB_BeamRevealPart")) ? 1 : 0;
            }
            TestTrue(TEXT("Ruin fragments are registered as individual reveal parts"), TaggedRevealParts >= 7);
            State->TaggedRevealPartCount = TaggedRevealParts;
            TActorIterator<ABBWeatherController> WeatherIt(World);
            ABBWeatherController* Weather = WeatherIt ? *WeatherIt : nullptr;
            TestNotNull(TEXT("Weather controller"), Weather);
            if (Weather)
            {
                State->Weather = Weather;
                State->RainFieldAnchor = Weather->RainRoot->GetComponentLocation();
                TestTrue(TEXT("Rain phase has fog density"), Weather->GetFogDensity() > 0.001f);
                TestTrue(TEXT("Volumetric fog is enabled"), Weather->FogComponent->bEnableVolumetricFog);
                USkyLightComponent* SkyFill = Weather->FindComponentByClass<USkyLightComponent>();
                TestNotNull(TEXT("Weather sky fill light"), SkyFill);
                TestTrue(TEXT("Runtime storm sky fill is movable"),
                    SkyFill && SkyFill->Mobility == EComponentMobility::Movable);
                TestTrue(TEXT("Runtime storm sky fill has nonzero intensity"),
                    SkyFill && SkyFill->Intensity > 0.0f);
                TestTrue(TEXT("Rain field anchor matches the weather actor, not the player"),
                    State->RainFieldAnchor.Equals(Weather->GetActorLocation(), 1.0f));
            }
            USpotLightComponent* BeamLight = State->Lighthouse->FindComponentByClass<USpotLightComponent>();
            TestNotNull(TEXT("Beam spotlight"), BeamLight);
            if (BeamLight)
            {
                TestTrue(TEXT("Beam contributes to volumetric fog"), BeamLight->VolumetricScatteringIntensity > 0.0f);
            }
            TestTrue(TEXT("Generator spawned at usable height in attached annex"), GeneratorActor->GetActorLocation().Equals(FVector(0.0f, -620.0f, 90.0f), 1.0f));
            TestTrue(TEXT("Anomaly spawned on far cliff"), State->Anomaly->GetActorLocation().Equals(FVector(-5200.0f, 4200.0f, 80.0f), 1.0f));
            TestTrue(TEXT("Player starts at landing"), FVector2D(State->Pawn->GetActorLocation()).Equals(FVector2D(-4500.0f, -600.0f), 10.0f));
            State->Pawn->SetActorLocation(FVector(360.0f, 0.0f, 100.0f));
            TestTrue(TEXT("Entering lighthouse volume completes objective"), Objectives->IsCompleted(TEXT("BB_OBJ_ENTER_LIGHTHOUSE")));
            State->Pawn->SetActorLocation(FVector(0.0f, -620.0f, 100.0f));
            TestTrue(TEXT("Entering annex volume finds generator"), Objectives->IsCompleted(TEXT("BB_OBJ_FIND_GENERATOR")));
            State->Pawn->SetActorLocation(FVector(-260.0f, -620.0f, 100.0f));
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
            State->GeneratorFlywheelStartRoll = State->GeneratorFlywheelPivot.IsValid()
                ? State->GeneratorFlywheelPivot->GetRelativeRotation().Roll : 0.0f;
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
            if (!State->bFlywheelMotionChecked && Now - State->StageAt >= 0.4)
            {
                TestTrue(TEXT("Flywheel visibly turns during generator spin-up"),
                    State->GeneratorFlywheelPivot.IsValid()
                    && !FMath::IsNearlyEqual(State->GeneratorFlywheelStartRoll,
                        State->GeneratorFlywheelPivot->GetRelativeRotation().Roll, 0.1f));
                State->bFlywheelMotionChecked = true;
            }
            if (!Generator->IsProducing())
            {
                TestFalse(TEXT("Power stays off during spin-up"), Lighthouse->IsPowered());
                return false;
            }
            TestTrue(TEXT("Generator start objective completed"), Objectives->IsCompleted(TEXT("BB_OBJ_START_GENERATOR")));
            TestTrue(TEXT("Restore power objective completed"), Objectives->IsCompleted(TEXT("BB_OBJ_RESTORE_POWER")));
            TestTrue(TEXT("Lighthouse has power"), Lighthouse->IsPowered());
            TestFalse(TEXT("Beam remains off before control interaction"), Lighthouse->BeamComponent->IsPowered());
            if (FApp::CanEverRender() && !State->GeneratorCaptureCamera.IsValid())
            {
                ACameraActor* Camera = World->SpawnActor<ACameraActor>();
                if (!Camera)
                {
                    AddError(TEXT("Could not create a generator visual probe camera"));
                    return true;
                }
                const FVector CameraLocation(-330.0f, -620.0f, 155.0f);
                const FVector GeneratorTarget(0.0f, -620.0f, 105.0f);
                Camera->SetActorLocation(CameraLocation);
                Camera->SetActorRotation((GeneratorTarget - CameraLocation).Rotation());
                State->GeneratorCaptureCamera = Camera;
                World->GetFirstPlayerController()->SetViewTarget(Camera);
                State->bGeneratorCameraReady = true;
                State->StageAt = Now;
                return false;
            }
            if (State->bGeneratorCameraReady && !State->bGeneratorCaptured)
            {
                if (Now - State->StageAt < 0.8)
                {
                    return false;
                }
                FScreenshotRequest::RequestScreenshot(TEXT("BlackBeacon_M02_Generator.png"), false, false);
                State->bGeneratorCaptured = true;
                State->StageAt = Now;
                return false;
            }
            if (State->bGeneratorCaptured && Now - State->StageAt < 0.3)
            {
                return false;
            }
            if (State->GeneratorCaptureCamera.IsValid())
            {
                World->GetFirstPlayerController()->SetViewTarget(State->Pawn.Get());
                State->GeneratorCaptureCamera->Destroy();
                State->GeneratorCaptureCamera.Reset();
            }
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
            const FVector ToAnomaly = GetRevealPartCenter(State->Anomaly.Get())
                - Lighthouse->BeamComponent->GetComponentLocation();
            Lighthouse->BeamComponent->SetManualYawTarget(FMath::RadiansToDegrees(FMath::Atan2(ToAnomaly.Y, ToAnomaly.X)));
            Lighthouse->BeamComponent->SetManualPitchDegrees(FMath::RadiansToDegrees(FMath::Atan2(ToAnomaly.Z, FVector2D(ToAnomaly.X, ToAnomaly.Y).Size())));
            ABBlackBeaconPlayerCharacter* Character = Cast<ABBlackBeaconPlayerCharacter>(State->Pawn.Get());
            if (FApp::CanEverRender() && Character && Character->GetController())
            {
                const FVector OpticsTarget = Lighthouse->GetActorLocation();
                Character->GetController()->SetControlRotation(
                    (OpticsTarget - Character->GetFirstPersonCamera()->GetComponentLocation()).Rotation());
                FScreenshotRequest::RequestScreenshot(
                    TEXT("BlackBeacon_Hero_D_LanternRoom.png"), false, false);
            }
            State->Stage = 4;
            State->StageAt = Now;
            return false;
        }
        if (State->Stage == 4 && Objectives->IsCompleted(TEXT("BB_OBJ_DISCOVER_ANOMALY")))
        {
            TestFalse(TEXT("The slice still has an objective after the reveal"), Objectives->IsFinished());
            TestEqual(TEXT("Discovery activates the approach objective"), Objectives->GetCurrentObjectiveId(),
                FString(TEXT("BB_OBJ_APPROACH_REVEAL")));
            TestFalse(TEXT("Anomaly is visible"), State->Anomaly->IsHidden());
            int32 VisibleRuinParts = 0;
            float TallestRuinPartExtent = 0.0f;
            UStaticMeshComponent* FirstRuinPart = nullptr;
			for (UStaticMeshComponent* Mesh : TInlineComponentArray<UStaticMeshComponent*>(State->Anomaly.Get()))
			{
				if (Mesh && Mesh->ComponentHasTag(TEXT("BB_BeamRevealPart")))
                {
                    if (!FirstRuinPart)
                    {
                        FirstRuinPart = Mesh;
                    }
                    TallestRuinPartExtent = FMath::Max(TallestRuinPartExtent, Mesh->Bounds.BoxExtent.Z);
                    VisibleRuinParts += Mesh->IsVisible() ? 1 : 0;
                }
            }
            UTexture2D* const WreckTexture = LoadObject<UTexture2D>(nullptr,
                TEXT("/Game/BlackBeacon/Textures/T_WreckHullAlbedo.T_WreckHullAlbedo"));
            UMaterialInstanceDynamic* const WreckMaterial = FirstRuinPart
                ? Cast<UMaterialInstanceDynamic>(FirstRuinPart->GetMaterial(0)) : nullptr;
            TestNotNull(TEXT("Wreck hull albedo asset loads"), WreckTexture);
            TestTrue(TEXT("The wreck material uses its imported hull albedo"),
                WreckMaterial && WreckTexture
                && WreckMaterial->K2_GetTextureParameterValue(TEXT("RockAlbedo")) == WreckTexture);
            TestTrue(TEXT("BeamReveal exposes pieces intersecting the moving beam"), VisibleRuinParts > 0);
            TestTrue(TEXT("Only beam-intersected ruin parts are visible"),
                VisibleRuinParts < State->TaggedRevealPartCount);
            TestTrue(TEXT("Ruin structure has a distant landmark silhouette"), TallestRuinPartExtent >= 800.0f);
            TestTrue(TEXT("The anomaly has completed its reveal transition"), State->AnomalyReveal->WasFullyRevealed());
            TestTrue(TEXT("The revealed fragment has full visible weight"), State->AnomalyReveal->GetVisibilityAmount() >= 0.99f);
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
                TActorIterator<ABBWeatherController> WeatherIt(World);
                if (WeatherIt)
                {
                    WeatherIt->SetWeather(EBBWeatherPhase::Storm, 0.0f);
                }
                State->RevealCaptureStartedAt = Now;
                State->Stage = 13;
                State->StageAt = Now;
                return false;
            }
            State->Stage = 7;
        }
        if (State->Stage == 13)
        {
            const auto Query = Lighthouse->BeamComponent->GetBeamQuery();
            UMaterialInstanceDynamic* ShaftMaterial = nullptr;
            for (UStaticMeshComponent* Mesh : TInlineComponentArray<UStaticMeshComponent*>(Lighthouse))
            {
                if (Mesh && Mesh->GetName() == TEXT("BeamVisualMesh"))
                {
                    ShaftMaterial = Cast<UMaterialInstanceDynamic>(Mesh->GetMaterial(0));
                    break;
                }
            }
            TestNotNull(TEXT("Visible beam shaft has a dynamic material"), ShaftMaterial);
            if (ShaftMaterial)
            {
                const float ShaftOpacity = ShaftMaterial->K2_GetScalarParameterValue(TEXT("BeamOpacity"));
                State->BeamShaftMaterial = ShaftMaterial;
                State->BeamOpacityBeforeDiagnostic = ShaftOpacity;
                const float ExpectedShaftOpacity = Lighthouse->BeamComponent->BeamVisualOpacity * Query.Intensity01;
                const float ShaftAngleTangent = ShaftMaterial->K2_GetScalarParameterValue(TEXT("BeamTanHalfAngle"));
                TestTrue(TEXT("Beam shaft material receives the live beam opacity"),
                    FMath::IsNearlyEqual(ShaftOpacity, ExpectedShaftOpacity,
                        FMath::Max(0.00000001f, ExpectedShaftOpacity * 0.05f)));
                TestTrue(TEXT("Beam shaft material width matches the gameplay cone"),
                    FMath::IsNearlyEqual(ShaftAngleTangent,
                        FMath::Tan(FMath::DegreesToRadians(Lighthouse->BeamComponent->BeamHalfAngleDeg)), 0.001f));
            }
            if (State->Weather.IsValid() && State->Weather->FogComponent)
            {
                const FLinearColor StormFogTint = State->Weather->FogComponent->FogInscatteringLuminance;
                TestTrue(TEXT("Storm fog keeps a dark blue-gray tint behind the warm beacon"),
                    StormFogTint.R <= 0.03f && StormFogTint.G <= 0.05f && StormFogTint.B <= 0.07f);
                TestTrue(TEXT("Storm fog retains its configured dense atmosphere"),
                    FMath::IsNearlyEqual(State->Weather->GetFogDensity(), 0.04f, 0.001f));
            }
            const FVector BeamDirection(Query.Direction.X, Query.Direction.Y, Query.Direction.Z);
            const FVector ToRuin = (GetRevealPartCenter(State->Anomaly.Get())
                - Lighthouse->BeamComponent->GetComponentLocation()).GetSafeNormal();
            const bool bBeamSettledOnRuin = FVector::DotProduct(BeamDirection, ToRuin) >= 0.995f;
            const bool bTimedOut = Now - State->RevealCaptureStartedAt >= 8.0;
            if (!bBeamSettledOnRuin && !bTimedOut)
            {
                return false;
            }
            TestTrue(TEXT("Beam settles on the anomaly before the reveal capture"), bBeamSettledOnRuin);

            State->CaptureCamera = World->SpawnActor<ACameraActor>();
            TestNotNull(TEXT("Fixed visual probe camera"), State->CaptureCamera.Get());
            if (!State->CaptureCamera.IsValid())
            {
                return true;
            }
            const FVector Side = FVector::CrossProduct(ToRuin, FVector::UpVector).GetSafeNormal();
			// Review the reveal from the coast side so the camera sees the beam
			// crossing the wreck instead of looking straight down the shaft.
			const FVector CameraLocation = GetRevealPartCenter(State->Anomaly.Get())
				- ToRuin * 6000.0f + Side * 6000.0f + FVector(0.0f, 0.0f, 400.0f);
            State->CaptureCamera->SetActorLocation(CameraLocation);
            State->CaptureCamera->SetActorRotation((GetRevealPartCenter(State->Anomaly.Get()) - CameraLocation).Rotation());
            if (State->CaptureCamera->GetCameraComponent())
            {
				State->CaptureCamera->GetCameraComponent()->SetFieldOfView(70.0f);
            }
            World->GetFirstPlayerController()->SetViewTarget(State->CaptureCamera.Get());
            State->CaptureIndex = 3;
            State->Stage = 6;
            State->StageAt = Now;
            return false;
        }
        if (State->Stage == 5)
        {
            const FVector Positions[] = {
                FVector(-4500.0f, -600.0f, 155.0f), // Shore-start player viewpoint
                FVector(-1200.0f, 1000.0f, 1700.0f),
                FVector(-4000.0f, 3300.0f, 900.0f),
                FVector(-8500.0f, 9500.0f, 2600.0f),
                FVector(-1200.0f, 1000.0f, 1700.0f), // Storm Dir 1
                FVector(-1200.0f, 1000.0f, 1700.0f), // Storm Dir 2
                FVector(-8200.0f, 6500.0f, 1300.0f),  // Storm Indoors (Reveal view)
                FVector(-1200.0f, 1000.0f, 1700.0f)  // Clear uses same position as B_Air
            };
            const FVector Targets[] = {
                FVector(0.0f, 0.0f, 1700.0f),
                FVector(-700.0f, 600.0f, 1700.0f),
                State->Anomaly->GetActorLocation(),
                GetRevealPartCenter(State->Anomaly.Get()),
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
            if (State->CaptureIndex == 3 && State->CaptureCamera->GetCameraComponent())
            {
                State->CaptureCamera->GetCameraComponent()->SetFieldOfView(60.0f);
            }
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
            if (State->CaptureIndex == 4 && !State->bBeamScatteringDiagnosticCaptured)
            {
                USpotLightComponent* BeamLight = State->Lighthouse.IsValid()
                    ? State->Lighthouse->FindComponentByClass<USpotLightComponent>() : nullptr;
                TestNotNull(TEXT("Beam spotlight for scattering comparison"), BeamLight);
                if (BeamLight)
                {
                    State->BeamScatteringBeforeDiagnostic = BeamLight->VolumetricScatteringIntensity;
                    BeamLight->SetVolumetricScatteringIntensity(0.0f);
                    FScreenshotRequest::RequestScreenshot(
                        TEXT("BlackBeacon_M01_D_Reveal_NoSpotScatter.png"), false, false);
                    State->Stage = 15;
                    State->StageAt = Now;
                    return false;
                }
            }
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
        if (State->Stage == 15 && Now - State->StageAt >= 0.8)
        {
            USpotLightComponent* BeamLight = State->Lighthouse.IsValid()
                ? State->Lighthouse->FindComponentByClass<USpotLightComponent>() : nullptr;
            TestNotNull(TEXT("Beam spotlight after scattering comparison"), BeamLight);
            if (BeamLight)
            {
                BeamLight->SetVolumetricScatteringIntensity(State->BeamScatteringBeforeDiagnostic);
                TestTrue(TEXT("Beam volumetric scattering is restored after comparison"),
                    FMath::IsNearlyEqual(BeamLight->VolumetricScatteringIntensity,
                        State->BeamScatteringBeforeDiagnostic, 0.001f));
            }
            State->bBeamScatteringDiagnosticCaptured = true;
            TestTrue(TEXT("Beam shaft material remains available for the isolated mesh comparison"),
                State->BeamShaftMaterial.IsValid());
            if (State->BeamShaftMaterial.IsValid())
            {
                State->BeamShaftMaterial->SetScalarParameterValue(TEXT("BeamOpacity"), 0.0000001f);
            }
            State->Stage = 16;
            State->StageAt = Now;
            return false;
        }
        if (State->Stage == 16 && Now - State->StageAt >= 0.8)
        {
            TestTrue(TEXT("Beam shaft material remains available for the high-opacity comparison"),
                State->BeamShaftMaterial.IsValid());
            if (State->BeamShaftMaterial.IsValid())
            {
                TestTrue(TEXT("Beam shaft material accepts the low-opacity comparison value"),
                    FMath::IsNearlyEqual(
                        State->BeamShaftMaterial->K2_GetScalarParameterValue(TEXT("BeamOpacity")), 0.0000001f, 0.00000001f));
                FScreenshotRequest::RequestScreenshot(
                    TEXT("BlackBeacon_M01_D_Reveal_ShaftOpacity0000001.png"), false, false);
            }
            State->Stage = 17;
            State->StageAt = Now;
            return false;
        }
        if (State->Stage == 17 && Now - State->StageAt >= 0.8)
        {
            TestTrue(TEXT("Beam shaft material remains available for the zero-opacity comparison"),
                State->BeamShaftMaterial.IsValid());
            if (State->BeamShaftMaterial.IsValid())
            {
                State->BeamShaftMaterial->SetScalarParameterValue(TEXT("BeamOpacity"), 0.0f);
            }
            State->Stage = 18;
            State->StageAt = Now;
            return false;
        }
        if (State->Stage == 18 && Now - State->StageAt >= 0.8)
        {
            TestTrue(TEXT("Beam shaft material remains available for the no-mesh capture"),
                State->BeamShaftMaterial.IsValid());
            if (State->BeamShaftMaterial.IsValid())
            {
                FScreenshotRequest::RequestScreenshot(
                    TEXT("BlackBeacon_M01_D_Reveal_NoShaftMesh.png"), false, false);
            }
            State->Stage = 19;
            State->StageAt = Now;
            return false;
        }
        if (State->Stage == 19 && Now - State->StageAt >= 0.8)
        {
            TestTrue(TEXT("Beam shaft material remains available to restore gameplay opacity"),
                State->BeamShaftMaterial.IsValid());
            if (State->BeamShaftMaterial.IsValid())
            {
                State->BeamShaftMaterial->SetScalarParameterValue(
                    TEXT("BeamOpacity"), State->BeamOpacityBeforeDiagnostic);
            }
            TestTrue(TEXT("Beam visual opacity is restored after comparisons"),
                State->BeamShaftMaterial.IsValid()
                && FMath::IsNearlyEqual(State->BeamShaftMaterial->K2_GetScalarParameterValue(TEXT("BeamOpacity")),
                    State->BeamOpacityBeforeDiagnostic, FMath::Max(0.00000001f,
                        State->BeamOpacityBeforeDiagnostic * 0.05f)));
            State->Stage = 8;
            State->StageAt = Now;
            return false;
        }
        if (State->Stage == 11 && State->RevealFadePhase == 0 && Now - State->StageAt >= 0.3)
        {
            TestTrue(TEXT("BeamReveal starts fading when the moving beam leaves"),
                State->AnomalyReveal.IsValid()
                && State->AnomalyReveal->GetVisibilityAmount() < State->RevealVisibilityBeforePowerLoss - 0.1f);
            State->RevealFadePhase = 1;
            State->StageAt = Now;
            return false;
        }
        if (State->Stage == 11 && State->RevealFadePhase == 1 && Now - State->StageAt >= 1.0)
        {
            TestTrue(TEXT("Ruin fragments disappear after the beam leaves"), State->Anomaly->IsHidden());
            TestTrue(TEXT("Discovery remains recorded after transient visuals fade"), State->AnomalyReveal->WasFullyRevealed());
            TestTrue(TEXT("Next objective remains active after the reveal fades"),
                Objectives->GetCurrentObjectiveId() == TEXT("BB_OBJ_APPROACH_REVEAL"));
            FScreenshotRequest::RequestScreenshot(TEXT("BlackBeacon_M01_D_RevealOff.png"), false, false);
            State->bRevealFadeChecked = true;
            State->Stage = 14;
            State->StageAt = Now;
            return false;
        }
        if (State->Stage == 14 && Now - State->StageAt >= 0.5)
        {
            State->Stage = 7;
            return false;
        }
        if (State->Stage == 12 && Now - State->StageAt >= 0.3)
        {
            TestTrue(TEXT("Flywheel coasts after generator shutdown"),
                State->GeneratorFlywheelPivot.IsValid()
                && !FMath::IsNearlyEqual(State->GeneratorFlywheelStopRoll,
                    State->GeneratorFlywheelPivot->GetRelativeRotation().Roll, 0.1f));
            State->bFlywheelCoastChecked = true;
            State->Stage = 7;
            return false;
        }
        if (State->Stage == 7)
        {
            if (!State->bRevealFadeChecked)
            {
                Lighthouse->BeamComponent->SetManualYawTarget(
                    Lighthouse->BeamComponent->GetCurrentYawDegrees() + 90.0f);
                State->RevealVisibilityBeforePowerLoss = State->AnomalyReveal.IsValid()
                    ? State->AnomalyReveal->GetVisibilityAmount() : 0.0f;
                State->RevealFadePhase = 0;
                State->Stage = 11;
                State->StageAt = Now;
                return false;
            }
            if (!State->bFlywheelCoastChecked)
            {
                Generator->Stop();
                State->GeneratorFlywheelStopRoll = State->GeneratorFlywheelPivot.IsValid()
                    ? State->GeneratorFlywheelPivot->GetRelativeRotation().Roll : 0.0f;
                State->Stage = 12;
                State->StageAt = Now;
                return false;
            }
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
                TestTrue(TEXT("Load resets transient reveal visuals"),
                    State->AnomalyReveal.IsValid() && State->AnomalyReveal->GetVisibilityAmount() <= 0.01f);
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
