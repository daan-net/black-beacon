#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "HAL/PlatformTime.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "BlackBeacon/Core/BBPlayerCharacter.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBBStairTraversalTest, "BlackBeacon.M01.StairTraversal",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FBBStairTraversalTest::RunTest(const FString& Parameters)
{
    struct FTraversalState
    {
        double StartedAt = FPlatformTime::Seconds();
        TWeakObjectPtr<ABBlackBeaconPlayerCharacter> Character;
        TArray<FVector> Steps;
        int32 NextStep = 1;
    };
    TSharedRef<FTraversalState> State = MakeShared<FTraversalState>();

    ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, State]()
    {
        if (FPlatformTime::Seconds() - State->StartedAt > 75.0)
        {
            const FVector Location = State->Character.IsValid() ? State->Character->GetActorLocation() : FVector::ZeroVector;
            const FVector Target = State->Steps.IsValidIndex(State->NextStep) ? State->Steps[State->NextStep] : FVector::ZeroVector;
            AddError(FString::Printf(TEXT("Stair traversal timed out at step %d of %d; pawn %s target %s"),
                State->NextStep, State->Steps.Num(), *Location.ToString(), *Target.ToString()));
            return true;
        }

        if (!State->Character.IsValid())
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
            if (!World || !World->GetFirstPlayerController())
            {
                return false;
            }
            State->Character = Cast<ABBlackBeaconPlayerCharacter>(World->GetFirstPlayerController()->GetPawn());
            TestNotNull(TEXT("Player character"), State->Character.Get());
            if (!State->Character.IsValid())
            {
                return true;
            }
            for (TActorIterator<AActor> It(World); It; ++It)
            {
                if (It->ActorHasTag(TEXT("BB_StairStep")))
                {
                    State->Steps.Add(It->GetActorLocation());
                }
            }
            State->Steps.Sort([](const FVector& A, const FVector& B) { return A.Z < B.Z; });
            TestEqual(TEXT("Greybox stair count"), State->Steps.Num(), 84);
            if (State->Steps.Num() != 84)
            {
                return true;
            }
            const FVector First = State->Steps[0];
            State->Character->SetActorLocation(FVector(First.X, First.Y, First.Z + 100.0f));
            return false;
        }

        ABBlackBeaconPlayerCharacter* Character = State->Character.Get();
        if (State->NextStep < State->Steps.Num())
        {
            const FVector Target = State->Steps[State->NextStep];
            const FVector Location = Character->GetActorLocation();
            if (FVector2D::Distance(FVector2D(Target), FVector2D(Location)) < 55.0f
                && Location.Z >= Target.Z + 65.0f)
            {
                ++State->NextStep;
            }
            else
            {
                const FVector Direction(Target.X - Location.X, Target.Y - Location.Y, 0.0f);
                Character->AddMovementInput(Direction.GetSafeNormal(), 1.0f);
            }
            return false;
        }

        TestTrue(TEXT("Pawn reached lantern height by walking stairs"), Character->GetActorLocation().Z > 1600.0f);
        TestTrue(TEXT("Pawn remains on walkable geometry"), Character->GetCharacterMovement()->IsMovingOnGround());
        return true;
    }));
    return true;
}

#endif
