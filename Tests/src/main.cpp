// BLACK BEACON - logic layer unit tests.
//
// Standalone tests for the engine-free Logics layer. Run with:
//   cmake -S Tests -B Tests/build && cmake --build Tests/build && Tests/build/bb_logic_tests
//
// No UE, no external test framework: plain asserts with a tiny harness.

#include <cmath>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include "BlackBeacon/Logics/BBBeamMath.h"
#include "BlackBeacon/Logics/BBObjectiveGraph.h"
#include "BlackBeacon/Logics/BBRevealStateMachine.h"
#include "BlackBeacon/Logics/BBWeatherState.h"

using namespace BlackBeacon::Logics;

namespace
{
	int g_Failures = 0;
	int g_Checks = 0;

	void Check(bool Condition, const std::string& What)
	{
		++g_Checks;
		if (!Condition)
		{
			++g_Failures;
			std::cerr << "FAIL: " << What << "\n";
		}
	}

	bool NearlyEqual(double A, double B, double Eps = 1e-6)
	{
		return std::fabs(A - B) <= Eps;
	}

	// ---------------------------------------------------------------
	// BeamMath
	// ---------------------------------------------------------------
	void TestBeamMath()
	{
		FBBBeamQuery Beam;
		Beam.Origin = BBVec3(0, 0, 0);
		Beam.Direction = BBVec3(0, 1, 0).Normalized(); // +Y
		Beam.HalfAngleRad = 0.105;                     // ~6 deg half cone
		Beam.Intensity01 = 1.0;
		Beam.RangeCm = 1000.0;
		Beam.bPowered = true;

		// Dead centre, near range: full-ish intensity.
		const double Centre = FBBBeamMath::ComputeConeIntensity(Beam, BBVec3(0, 500, 0));
		Check(NearlyEqual(Centre, 1.0), "beam_math: centre intensity ~1.0");

		// Inside the cone but near the edge: strictly less than centre.
		const double OffCentre = FBBBeamMath::ComputeConeIntensity(Beam, BBVec3(40, 500, 0));
		Check(OffCentre > 0.0 && OffCentre < Centre, "beam_math: angular falloff between 0 and centre");

		// Outside the cone: zero.
		const double Outside = FBBBeamMath::ComputeConeIntensity(Beam, BBVec3(400, 500, 0));
		Check(NearlyEqual(Outside, 0.0), "beam_math: outside cone is zero");

		// Beyond range: zero.
		const double Beyond = FBBBeamMath::ComputeConeIntensity(Beam, BBVec3(0, 5000, 0));
		Check(NearlyEqual(Beyond, 0.0), "beam_math: beyond range is zero");

		// Unpowered: zero everywhere.
		Beam.bPowered = false;
		Check(NearlyEqual(FBBBeamMath::ComputeConeIntensity(Beam, BBVec3(0, 500, 0)), 0.0),
			"beam_math: unpowered beam lights nothing");
		Beam.bPowered = true;

		// Zero intensity: zero everywhere.
		Beam.Intensity01 = 0.0;
		Check(NearlyEqual(FBBBeamMath::ComputeConeIntensity(Beam, BBVec3(0, 500, 0)), 0.0),
			"beam_math: zero intensity lights nothing");
		Beam.Intensity01 = 1.0;

		// In-cone predicate agrees with intensity gate.
		Check(FBBBeamMath::IsPointInBeamCone(Beam, BBVec3(0, 800, 0)), "beam_math: in-cone predicate true");
		Check(!FBBBeamMath::IsPointInBeamCone(Beam, BBVec3(900, 800, 0)), "beam_math: out-of-cone predicate false");
		Beam.bPowered = false;
		Check(!FBBBeamMath::IsPointInBeamCone(Beam, BBVec3(0, 800, 0)), "beam_math: unpowered predicate false");
	}

	// ---------------------------------------------------------------
	// RevealStateMachine
	// ---------------------------------------------------------------
	void TestRevealMachine()
	{
		FBBRevealParams Params;
		Params.RevealDelay = 2.0;
		Params.FadeTime = 1.0;
		Params.MinBeamIntensity = 0.25;
		Params.VisibilityDuration = 0.0; // visible only while illuminated
		Params.bPersistent = false;

		FBBRevealMachine Machine;
		Machine.SetParams(Params);

		const BBVec3 ObjPos(0, 500, 0);

		FBBBeamQuery Beam;
		Beam.Origin = BBVec3(0, 0, 0);
		Beam.Direction = BBVec3(0, 1, 0);
		Beam.Intensity01 = 1.0;
		Beam.RangeCm = 1000.0;
		Beam.bPowered = true;

		// 1. Starts hidden.
		Check(Machine.GetPhase() == EBBRevealPhase::Hidden, "reveal: starts hidden");
		Check(NearlyEqual(Machine.GetVisibilityAmount(), 0.0), "reveal: starts at 0 visibility");

		// 2. Beam rests for RevealDelay; still hidden during the delay.
		for (int I = 0; I < 19; ++I)
		{
			Machine.Tick(0.1, Beam, ObjPos);
		}
		Check(Machine.GetPhase() == EBBRevealPhase::Hidden, "reveal: still hidden before delay elapses");

		// 3. Past the delay it begins revealing.
		Machine.Tick(0.1, Beam, ObjPos);
		Check(Machine.GetPhase() == EBBRevealPhase::Revealing, "reveal: revealing after delay");

		// 4. Another FadeTime worth of ticks reaches full visibility.
		for (int I = 0; I < 10; ++I)
		{
			Machine.Tick(0.1, Beam, ObjPos);
		}
		Check(Machine.GetPhase() == EBBRevealPhase::Visible, "reveal: fully visible after fade");
		Check(NearlyEqual(Machine.GetVisibilityAmount(), 1.0), "reveal: amount == 1");
		Check(Machine.WasFullyRevealed(), "reveal: was-fully-revealed flag set");

		// 5. Beam leaves -> fades out over FadeTime.
		Beam.bPowered = false; // simulate beam turned away/off
		Machine.Tick(0.1, Beam, ObjPos);
		Check(Machine.GetPhase() == EBBRevealPhase::Fading, "reveal: fading when beam leaves");
		for (int I = 0; I < 10; ++I)
		{
			Machine.Tick(0.1, Beam, ObjPos);
		}
		Check(Machine.GetPhase() == EBBRevealPhase::Hidden, "reveal: hidden again after fade-out");
		Check(NearlyEqual(Machine.GetVisibilityAmount(), 0.0), "reveal: amount back to 0");

		// 6. Persistent reveals stay after a full reveal even without light.
		Params.bPersistent = true;
		Machine.Reset();
		Machine.SetParams(Params);
		Beam.bPowered = true;
		for (int I = 0; I < 40; ++I)
		{
			Machine.Tick(0.1, Beam, ObjPos);
		}
		Check(Machine.GetPhase() == EBBRevealPhase::Visible, "reveal: persistent reaches visible");
		Beam.bPowered = false;
		for (int I = 0; I < 20; ++I)
		{
			Machine.Tick(0.1, Beam, ObjPos);
		}
		Check(Machine.GetPhase() == EBBRevealPhase::Visible && NearlyEqual(Machine.GetVisibilityAmount(), 1.0),
			"reveal: persistent stays visible without light");

		// 7. A beam below MinBeamIntensity never reveals.
		Params.bPersistent = false;
		Machine.Reset();
		Machine.SetParams(Params);
		Beam.bPowered = true;
		Beam.Intensity01 = 0.1; // below MinBeamIntensity 0.25
		for (int I = 0; I < 100; ++I)
		{
			Machine.Tick(0.1, Beam, ObjPos);
		}
		Check(Machine.GetPhase() == EBBRevealPhase::Hidden, "reveal: dim beam never reveals");
		Check(!Machine.WasFullyRevealed(), "reveal: dim beam never fully reveals");
	}

	// ---------------------------------------------------------------
	// ObjectiveGraph
	// ---------------------------------------------------------------
	void TestObjectiveGraph()
	{
		FBBObjectiveGraph Graph;
		Graph.Configure({
			{"BB_OBJ_ARRIVE", "Arrive", {}},
			{"BB_OBJ_ENTER", "Enter the lighthouse", {"BB_OBJ_ARRIVE"}},
			{"BB_OBJ_FIND", "Find the generator", {"BB_OBJ_ENTER"}},
			{"BB_OBJ_START", "Start the generator", {"BB_OBJ_FIND"}},
		});

		// Chain starts with the first node active.
		Check(Graph.GetCurrentObjectiveId() == "BB_OBJ_ARRIVE", "obj: first node is current");
		Check(!Graph.IsFinished(), "obj: not finished");

		// Later nodes are locked.
		Check(Graph.ActivateObjective("BB_OBJ_ENTER") == false, "obj: prereq-locked node cannot activate");

		// Completing current auto-activates the next.
		Check(Graph.CompleteObjective("BB_OBJ_ARRIVE"), "obj: completing active node succeeds");
		Check(Graph.GetCurrentObjectiveId() == "BB_OBJ_ENTER", "obj: next node becomes current");

		// Cannot complete an inactive/non-current node.
		Check(Graph.CompleteObjective("BB_OBJ_START") == false, "obj: skipping is rejected");

		// Completing the chain finishes the graph.
		Check(Graph.CompleteObjective("BB_OBJ_ENTER"), "obj: enter completes");
		Check(Graph.CompleteObjective("BB_OBJ_FIND"), "obj: find completes");
		Check(Graph.CompleteObjective("BB_OBJ_START"), "obj: start completes");
		Check(Graph.IsFinished(), "obj: graph finished");
		Check(Graph.GetCurrentObjectiveId().empty(), "obj: no current objective when done");

		// Reset restores the initial state.
		Graph.Reset();
		Check(Graph.GetCurrentObjectiveId() == "BB_OBJ_ARRIVE", "obj: reset returns to start");
	}

	// ---------------------------------------------------------------
	// WeatherInterpolator
	// ---------------------------------------------------------------
	void TestWeather()
	{
		FBBWeatherInterpolator Weather;
		Weather.Configure({
			{EBBWeatherPhase::Clear, 0.0008, 0.0, 0.0, 0.15, 0.30, 0.32, 0.35},
			{EBBWeatherPhase::Fog, 0.0040, 0.0, 0.0, 0.65, 0.22, 0.24, 0.27},
			{EBBWeatherPhase::Rain, 0.0015, 0.45, 0.60, 0.80, 0.18, 0.20, 0.23},
			{EBBWeatherPhase::Storm, 0.0030, 0.95, 1.0, 0.95, 0.12, 0.14, 0.17},
		});

		Check(NearlyEqual(Weather.GetFogDensity(), 0.0008), "weather: starts at clear fog density");
		Check(NearlyEqual(Weather.GetWindStrength(), 0.0), "weather: starts calm");

		// Transition to Storm over 10 seconds; after 5s we are halfway.
		Weather.SetTarget(EBBWeatherPhase::Storm, 10.0);
		Weather.Tick(5.0);
		Check(Weather.IsTransitioning(), "weather: still transitioning at halfway");
		Check(NearlyEqual(Weather.GetWindStrength(), 0.475, 1e-3), "weather: halfway wind");
		Check(Weather.GetFogDensity() > 0.0008 && Weather.GetFogDensity() < 0.0030,
			"weather: fog density strictly interpolating");

		// Finish the transition; settle exactly on target.
		Weather.Tick(5.0);
		Check(!Weather.IsTransitioning(), "weather: settled");
		Check(NearlyEqual(Weather.GetWindStrength(), 0.95), "weather: storm wind reached");
		Check(NearlyEqual(Weather.GetRainIntensity(), 1.0), "weather: storm rain reached");
		Check(NearlyEqual(Weather.GetFogDensity(), 0.0030), "weather: storm fog density reached");

		// Instant transition snaps immediately.
		Weather.SetTarget(EBBWeatherPhase::Clear, 0.0);
		Check(NearlyEqual(Weather.GetFogDensity(), 0.0008), "weather: instant transition snaps");
	}
}

int main()
{
	TestBeamMath();
	TestRevealMachine();
	TestObjectiveGraph();
	TestWeather();

	std::cout << (g_Failures == 0 ? "ALL PASS" : "FAILURES") << "  -  "
		<< g_Checks - g_Failures << "/" << g_Checks << " checks passed\n";
	return g_Failures == 0 ? 0 : 1;
}