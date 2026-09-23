// BLACK BEACON - Logics: weather state interpolation.
//
// The weather controller's transition logic as pure C++: a palette of
// per-phase targets (fog density/colour, wind, rain, cloudiness) and a
// timed interpolation between phases. Engine-free so it is unit-tested
// standalone; UBBWeatherController is a thin adapter driving
// ExponentialHeightFog and (later) Niagara.

#pragma once

#include <cstdint>
#include <vector>

namespace BlackBeacon::Logics
{
	enum class EBBWeatherPhase : std::uint8_t
	{
		Clear = 0,
		Fog,
		Rain,
		Storm,
		Count
	};

	struct FBBWeatherPaletteEntry
	{
		EBBWeatherPhase Phase = EBBWeatherPhase::Clear;
		double FogDensity = 0.0008;     // exponential height fog density
		double WindStrength = 0.0;      // 0..1 (vegetation/particles later)
		double RainIntensity = 0.0;     // 0..1 output for the rendered rain field
		double Cloudiness = 0.15;       // 0..1 (sky light/shadow softness later)
		double FogR = 0.30;             // fog inscattering colour (linear)
		double FogG = 0.32;
		double FogB = 0.35;
	};

	class FBBWeatherInterpolator
	{
	public:
		// Palette must have at most EBBWeatherPhase::Count entries; missing
		// phases keep the defaults present in the struct.
		void Configure(const std::vector<FBBWeatherPaletteEntry>& Palette);

		// Begin transitioning to Phase over TransitionSeconds.
		void SetTarget(EBBWeatherPhase Phase, double TransitionSeconds);

		void Tick(double DeltaSeconds);

		EBBWeatherPhase GetTargetPhase() const { return TargetPhase; }
		bool IsTransitioning() const { return SecondsRemaining > 0.0; }

		double GetFogDensity() const { return Vars.FogDensity; }
		double GetWindStrength() const { return Vars.WindStrength; }
		double GetRainIntensity() const { return Vars.RainIntensity; }
		double GetCloudiness() const { return Vars.Cloudiness; }
		void GetFogColor(double& OutR, double& OutG, double& OutB) const
		{
			OutR = Vars.FogR; OutG = Vars.FogG; OutB = Vars.FogB;
		}

	private:
		struct FVars
		{
			double FogDensity = 0.0008;
			double WindStrength = 0.0;
			double RainIntensity = 0.0;
			double Cloudiness = 0.15;
			double FogR = 0.30, FogG = 0.32, FogB = 0.35;
		};

		FVars Vars;
		FVars StartVars;
		std::vector<FBBWeatherPaletteEntry> Palette;
		EBBWeatherPhase TargetPhase = EBBWeatherPhase::Clear;
		double TransitionSeconds = 1.0;
		double SecondsRemaining = 0.0;

		void SnapTo(const FBBWeatherPaletteEntry& Entry);
		void Snapshot(FVars& Out) const;
		void ApplyLerp(double Alpha);
	};

	inline const FBBWeatherPaletteEntry& EntryForPhase(
		const std::vector<FBBWeatherPaletteEntry>& Palette,
		EBBWeatherPhase Phase)
	{
		for (const FBBWeatherPaletteEntry& Entry : Palette)
		{
			if (Entry.Phase == Phase)
			{
				return Entry;
			}
		}
		// Caller guards: palette is indexed by phase in Configure.
		static const FBBWeatherPaletteEntry Fallback{};
		return Fallback;
	}
}
