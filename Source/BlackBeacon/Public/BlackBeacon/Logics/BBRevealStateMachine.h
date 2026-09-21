// BLACK BEACON - Logics: beam-reveal state machine.
//
// The signature anomaly behaviour: an object is invisible normally; the
// lighthouse beam rests on it, and it materialises. Leave, and it fades
// (or stays, if the reveal is persistent).
//
// Pure engine-free C++ so the timing behaviour is unit-tested in Tests/
// without the engine. The UE component (UBBBeamRevealComponent) is a thin
// adapter over this machine plus visibility/mesh handling.

#pragma once

#include <cstdint>

#include "BlackBeacon/Logics/BBBeamMath.h"

namespace BlackBeacon::Logics
{
	enum class EBBRevealPhase : std::uint8_t
	{
		Hidden,     // nothing visible
		Revealing,  // rising fade-in at FadeTime rate
		Visible,    // fully revealed
		Fading      // falling fade-out at FadeTime rate
	};

	// Data-driven reveal behaviour. All fields map to config.
	struct FBBRevealParams
	{
		double RevealDelay = 2.0;     // seconds the beam must rest first
		double FadeTime = 1.0;        // seconds for full fade in/out
		double MinBeamIntensity = 0.25; // minimum ComputeConeIntensity to count
		double VisibilityDuration = 0.0; // seconds to HOLD full visibility (0 = while illuminated)
		bool bPersistent = false;     // once revealed, stays revealed forever
	};

	class FBBRevealMachine
	{
	public:
		// Advance the machine by DeltaSeconds against the current beam state.
		// ObjectPosition is the reveal object's centre in world space.
		void Tick(double DeltaSeconds, const FBBBeamQuery& Beam, const BBVec3& ObjectPosition);

		EBBRevealPhase GetPhase() const { return Phase; }
		// 0..1 visibility amount used to drive hidden state / material fade.
		double GetVisibilityAmount() const { return VisibilityAmount; }
		// True once the object has ever reached full visibility. Used for
		// one-shot objectives/story triggers.
		bool WasFullyRevealed() const { return bWasFullyRevealed; }

		void SetParams(const FBBRevealParams& InParams) { Params = InParams; }
		const FBBRevealParams& GetParams() const { return Params; }

		void Reset();

	private:
		bool IsIlluminated(const FBBBeamQuery& Beam, const BBVec3& ObjectPosition) const;

		FBBRevealParams Params;
		EBBRevealPhase Phase = EBBRevealPhase::Hidden;
		double VisibilityAmount = 0.0;
		double IlluminationTime = 0.0;   // continuous time beam has rested (post-delay)
		double VisibleHoldTime = 0.0;    // time spent holding full visibility
		bool bWasFullyRevealed = false;
		bool bIlluminatedLastTick = false;
	};
}