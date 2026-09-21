#include "BlackBeacon/Logics/BBWeatherState.h"

namespace BlackBeacon::Logics
{
	void FBBWeatherInterpolator::Configure(const std::vector<FBBWeatherPaletteEntry>& InPalette)
	{
		Palette = InPalette;
		if (Palette.size() > static_cast<std::size_t>(EBBWeatherPhase::Count))
		{
			Palette.resize(static_cast<std::size_t>(EBBWeatherPhase::Count));
		}
		const FBBWeatherPaletteEntry& Start = EntryForPhase(Palette, EBBWeatherPhase::Clear);
		SnapTo(Start);
	}

	void FBBWeatherInterpolator::SetTarget(EBBWeatherPhase Phase, double InTransitionSeconds)
	{
		Snapshot(StartVars);
		TargetPhase = Phase;
		TransitionSeconds = InTransitionSeconds > 0.0 ? InTransitionSeconds : 0.0;
		SecondsRemaining = TransitionSeconds;
		if (SecondsRemaining <= 0.0)
		{
			SnapTo(EntryForPhase(Palette, TargetPhase));
		}
	}

	void FBBWeatherInterpolator::Tick(double DeltaSeconds)
	{
		if (SecondsRemaining <= 0.0)
		{
			return; // settled
		}

		SecondsRemaining -= DeltaSeconds;
		const double Alpha = SecondsRemaining <= 0.0
			? 1.0
			: 1.0 - SecondsRemaining / TransitionSeconds;
		ApplyLerp(Alpha);

		if (SecondsRemaining <= 0.0)
		{
			SnapTo(EntryForPhase(Palette, TargetPhase)); // remove drift
		}
	}

	void FBBWeatherInterpolator::SnapTo(const FBBWeatherPaletteEntry& Entry)
	{
		Vars.FogDensity = Entry.FogDensity;
		Vars.WindStrength = Entry.WindStrength;
		Vars.RainIntensity = Entry.RainIntensity;
		Vars.Cloudiness = Entry.Cloudiness;
		Vars.FogR = Entry.FogR;
		Vars.FogG = Entry.FogG;
		Vars.FogB = Entry.FogB;
	}

	void FBBWeatherInterpolator::Snapshot(FVars& Out) const
	{
		Out = Vars;
	}

	void FBBWeatherInterpolator::ApplyLerp(double Alpha)
	{
		const FBBWeatherPaletteEntry& Target = EntryForPhase(Palette, TargetPhase);
		Vars.FogDensity = StartVars.FogDensity + (Target.FogDensity - StartVars.FogDensity) * Alpha;
		Vars.WindStrength = StartVars.WindStrength + (Target.WindStrength - StartVars.WindStrength) * Alpha;
		Vars.RainIntensity = StartVars.RainIntensity + (Target.RainIntensity - StartVars.RainIntensity) * Alpha;
		Vars.Cloudiness = StartVars.Cloudiness + (Target.Cloudiness - StartVars.Cloudiness) * Alpha;
		Vars.FogR = StartVars.FogR + (Target.FogR - StartVars.FogR) * Alpha;
		Vars.FogG = StartVars.FogG + (Target.FogG - StartVars.FogG) * Alpha;
		Vars.FogB = StartVars.FogB + (Target.FogB - StartVars.FogB) * Alpha;
	}
}