#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputKeyEventArgs.h"
#include "HAL/PlatformTime.h"
#include "InputCoreTypes.h"

#include "BlackBeacon/Core/BBPlayerCharacter.h"
#include "BlackBeacon/Core/BBPlayerController.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBBPlayerControlsTest, "BlackBeacon.M01.PlayerControls",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

namespace
{
    void SendKey(ABBlackBeaconPlayerController* Controller, FKey Key, EInputEvent Event, float Amount)
    {
        Controller->InputKey(FInputKeyEventArgs(nullptr, INPUTDEVICEID_NONE, Key, Event,
            Amount, false, FPlatformTime::Cycles64()));
    }
}

bool FBBPlayerControlsTest::RunTest(const FString& Parameters)
{
    struct FControlState
    {
        int32 Stage = 0;
        double StartedAt = FPlatformTime::Seconds();
        double StageAt = StartedAt;
        TWeakObjectPtr<ABBlackBeaconPlayerController> Controller;
        TWeakObjectPtr<ABBlackBeaconPlayerCharacter> Character;
        FVector StartLocation = FVector::ZeroVector;
        float StartYaw = 0.0f;
        float StartPitch = 0.0f;
    };
    TSharedRef<FControlState> State = MakeShared<FControlState>();

    ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, State]()
    {
        const double Now = FPlatformTime::Seconds();
        if (Now - State->StartedAt > 10.0)
        {
            AddError(TEXT("Player control input timed out"));
            return true;
        }
        if (State->Stage == 0)
        {
            UWorld* World = nullptr;
            for (const FWorldContext& Context : GEngine->GetWorldContexts())
            {
                if (Context.WorldType == EWorldType::Game && Context.World() && Context.World()->HasBegunPlay())
                {
                    World = Context.World();
                    break;
                }
            }
            if (!World)
            {
                return false;
            }
            State->Controller = Cast<ABBlackBeaconPlayerController>(World->GetFirstPlayerController());
            State->Character = State->Controller.IsValid() ? Cast<ABBlackBeaconPlayerCharacter>(State->Controller->GetPawn()) : nullptr;
            TestNotNull(TEXT("BlackBeacon controller"), State->Controller.Get());
            TestNotNull(TEXT("BlackBeacon character"), State->Character.Get());
            if (!State->Controller.IsValid() || !State->Character.IsValid())
            {
                return true;
            }
            State->StartLocation = State->Character->GetActorLocation();
            State->StartYaw = State->Controller->GetControlRotation().Yaw;
            const float FootZ = State->Character->GetActorLocation().Z
                - State->Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
            const float EyeHeight = State->Character->GetFirstPersonCamera()->GetComponentLocation().Z - FootZ;
            TestTrue(TEXT("Standing eye height is human scale"), EyeHeight > 140.0f && EyeHeight < 165.0f);
            SendKey(State->Controller.Get(), EKeys::W, IE_Pressed, 1.0f);
            SendKey(State->Controller.Get(), EKeys::LeftShift, IE_Pressed, 1.0f);
            State->Stage = 1;
            State->StageAt = Now;
            return false;
        }
        ABBlackBeaconPlayerController* Controller = State->Controller.Get();
        ABBlackBeaconPlayerCharacter* Character = State->Character.Get();
        if (!Controller || !Character)
        {
            AddError(TEXT("Player was destroyed during input test"));
            return true;
        }
        if (State->Stage == 1 && Now - State->StageAt >= 0.5)
        {
            SendKey(Controller, EKeys::W, IE_Released, 0.0f);
            SendKey(Controller, EKeys::LeftShift, IE_Released, 0.0f);
            TestTrue(TEXT("W moves the pawn"), FVector::Dist2D(Character->GetActorLocation(), State->StartLocation) > 20.0f);
            TestTrue(TEXT("Shift starts sprint"), Character->IsSprinting());
            SendKey(Controller, EKeys::C, IE_Pressed, 1.0f);
            State->Stage = 2;
            State->StageAt = Now;
            return false;
        }
        if (State->Stage == 2 && Now - State->StageAt >= 0.15)
        {
            SendKey(Controller, EKeys::C, IE_Released, 0.0f);
            TestTrue(TEXT("C toggles crouch"), Character->IsCrouchedByPlayer());
            TestTrue(TEXT("Crouch lowers capsule"), Character->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() < 80.0f);
            SendKey(Controller, EKeys::MouseX, IE_Axis, 20.0f);
            State->Stage = 3;
            State->StageAt = Now;
            return false;
        }
        if (State->Stage == 3 && Now - State->StageAt >= 0.15)
        {
            const float YawDelta = FMath::FindDeltaAngleDegrees(State->StartYaw, Controller->GetControlRotation().Yaw);
            TestTrue(TEXT("Mouse yaw is responsive but controllable"), YawDelta > 0.1f && YawDelta < 10.0f);
            State->StartPitch = Controller->GetControlRotation().Pitch;
            SendKey(Controller, EKeys::MouseY, IE_Axis, 20.0f);
            State->Stage = 4;
            State->StageAt = Now;
            return false;
        }
        if (State->Stage == 4 && Now - State->StageAt >= 0.15)
        {
            const float PitchDelta = FMath::FindDeltaAngleDegrees(State->StartPitch, Controller->GetControlRotation().Pitch);
            TestTrue(TEXT("Mouse down looks down at a controllable rate"), PitchDelta < -0.1f && PitchDelta > -10.0f);
            return true;
        }
        return false;
    }));
    return true;
}

#endif
