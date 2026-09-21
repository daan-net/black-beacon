#include "BlackBeacon/Logics/BBRevealStateMachine.h"

#include <algorithm>

namespace BlackBeacon::Logics
{
	bool FBBRevealMachine::IsIlluminated(const FBBBeamQuery& Beam, const BBVec3& ObjectPosition) const
	{
		return Beam.bPowered
			&& FBBBeamMath::ComputeConeIntensity(Beam, ObjectPosition) >= Params.MinBeamIntensity;
	}

	void FBBRevealMachine::Reset()
	{
		Phase = EBBRevealPhase::Hidden;
		VisibilityAmount = 0.0;
		IlluminationTime = 0.0;
		VisibleHoldTime = 0.0;
		bWasFullyRevealed = false;
		bIlluminatedLastTick = false;
	}

	void FBBRevealMachine::Tick(double DeltaSeconds, const FBBBeamQuery& Beam, const BBVec3& ObjectPosition)
	{
		if (DeltaSeconds <= 0.0)
		{
			return;
		}

		const bool bLit = IsIlluminated(Beam, ObjectPosition);

		// Persistent reveals that have already been fully revealed never fade.
		if (Params.bPersistent && bWasFullyRevealed)
		{
			VisibilityAmount = 1.0;
			Phase = EBBRevealPhase::Visible;
			IlluminationTime = Params.RevealDelay; // keep "rested" state hot
			bIlluminatedLastTick = bLit || bWasFullyRevealed;
			return;
		}

		// --- illumination accumulates ---
		if (bLit)
		{
			IlluminationTime += DeltaSeconds;
		}
		else
		{
			// The beam is no longer resting: decay the rested-time at the
			// fade rate so a brief flicker doesn't reset all progress.
			IlluminationTime = std::max(0.0, IlluminationTime - DeltaSeconds);
		}

		// --- full visibility hold timer ---
		if (Phase == EBBRevealPhase::Visible && Params.VisibilityDuration > 0.0)
		{
			if (bLit)
			{
				VisibleHoldTime += DeltaSeconds;
				if (VisibleHoldTime >= Params.VisibilityDuration)
				{
					// Held long enough; fall into fading when the beam leaves.
					Phase = EBBRevealPhase::Fading;
				}
			}
			else
			{
				Phase = EBBRevealPhase::Fading;
			}
		}

		// --- transitions ---
		switch (Phase)
		{
		case EBBRevealPhase::Hidden:
			if (bLit && IlluminationTime >= Params.RevealDelay)
			{
				Phase = EBBRevealPhase::Revealing;
			}
			break;

		case EBBRevealPhase::Revealing:
			if (!bLit && VisibilityAmount <= 0.0)
			{
				Phase = EBBRevealPhase::Hidden;
			}
			else if (!bLit)
			{
				Phase = EBBRevealPhase::Fading;
			}
			break;

		case EBBRevealPhase::Visible:
			if (!bLit && Params.VisibilityDuration <= 0.0)
			{
				// No explicit hold duration: visible only while illuminated.
				Phase = EBBRevealPhase::Fading;
			}
			else if (Params.VisibilityDuration > 0.0 && VisibleHoldTime >= Params.VisibilityDuration)
			{
				Phase = EBBRevealPhase::Fading;
			}
			break;

		case EBBRevealPhase::Fading:
			break; // handled by the amount lerp below
		}

		// --- visibility amount ---
		switch (Phase)
		{
		case EBBRevealPhase::Hidden:
			VisibilityAmount = 0.0;
			break;
		case EBBRevealPhase::Revealing:
			VisibilityAmount = std::min(1.0, VisibilityAmount + DeltaSeconds / Params.FadeTime);
			if (VisibilityAmount >= 1.0)
			{
				VisibilityAmount = 1.0;
				Phase = EBBRevealPhase::Visible;
				bWasFullyRevealed = true;
				VisibleHoldTime = 0.0;
			}
			break;
		case EBBRevealPhase::Visible:
			VisibilityAmount = 1.0;
			break;
		case EBBRevealPhase::Fading:
			VisibilityAmount = std::max(0.0, VisibilityAmount - DeltaSeconds / Params.FadeTime);
			if (VisibilityAmount <= 0.0)
			{
				VisibilityAmount = 0.0;
				Phase = EBBRevealPhase::Hidden;
			}
			break;
		}

		bIlluminatedLastTick = bLit;
	}
}
