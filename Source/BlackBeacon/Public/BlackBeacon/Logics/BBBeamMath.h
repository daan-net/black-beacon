// BLACK BEACON - Logics: beam math.
//
// Pure, engine-free math for the lighthouse beam (the signature system).
// No UE types here on purpose: this file is unit-tested standalone in
// Tests/ and compiled into the runtime module by Unreal Build Tool.

#pragma once

#include <cmath>

namespace BlackBeacon::Logics
{
	// Minimal 3D vector used by the logic layer. The UE adapters convert
	// to/from FVector at the system boundary.
	struct BBVec3
	{
		double X = 0.0;
		double Y = 0.0;
		double Z = 0.0;

		BBVec3() = default;
		BBVec3(double InX, double InY, double InZ) : X(InX), Y(InY), Z(InZ) {}

		BBVec3 operator+(const BBVec3& O) const { return BBVec3(X + O.X, Y + O.Y, Z + O.Z); }
		BBVec3 operator-(const BBVec3& O) const { return BBVec3(X - O.X, Y - O.Y, Z - O.Z); }
		BBVec3 operator*(double S) const { return BBVec3(X * S, Y * S, Z * S); }
		BBVec3 operator/(double S) const { return BBVec3(X / S, Y / S, Z / S); }
		BBVec3 operator-() const { return BBVec3(-X, -Y, -Z); }

		double Dot(const BBVec3& O) const { return X * O.X + Y * O.Y + Z * O.Z; }
		double LengthSq() const { return Dot(*this); }
		double Length() const { return std::sqrt(LengthSq()); }

		// Returns a normalized copy. If the vector is (near) zero, returns
		// (0,0,1) as a safe default so downstream math never degenerates.
		BBVec3 Normalized() const
		{
			const double Len = Length();
			if (Len > 1e-9)
			{
				return *this / Len;
			}
			return BBVec3(0.0, 0.0, 1.0);
		}
	};

	// A snapshot of the beam state that reveal/anomaly systems consume.
	// This is the reusable seam between "the beam" and "everything the
	// beam reveals" - consumers never reach into light components.
	struct FBBBeamQuery
	{
		BBVec3  Origin;          // world-space lamp origin (cm)
		BBVec3  Direction;       // normalized, world-space
		double  HalfAngleRad = 0.105; // cone half-angle (~6 degrees)
		double  Intensity01 = 1.0;    // current intensity, 0..1 (post lerp/flicker/power)
		double  RangeCm = 180000.0;   // how far the cone meaningfully reads
		bool    bPowered = true;
        bool bDiscoveryEnabled = true; // Runtime beam enables discovery only after manual aim input.
	};

	class FBBBeamMath
	{
	public:
		// True when Point is inside the beam cone (angular + range gated)
		// and the beam is powered with usable intensity.
		static bool IsPointInBeamCone(const FBBBeamQuery& Beam, const BBVec3& Point);

		// 0..1: how strongly the beam rests on Point. Uses a cosine
		// (smoothstep-like) angular falloff; range is a hard gate. Returns
		// 0 when unpowered, out of range, or outside the cone, regardless
		// of configured minimums - consumers apply their own thresholds.
		static double ComputeConeIntensity(const FBBBeamQuery& Beam, const BBVec3& Point);
	};

	inline double Clamp(double V, double Lo, double Hi)
	{
		return V < Lo ? Lo : (V > Hi ? Hi : V);
	}

	inline double SmoothStep(double Edge0, double Edge1, double V)
	{
		const double T = Clamp((V - Edge0) / (Edge1 - Edge0), 0.0, 1.0);
		return T * T * (3.0 - 2.0 * T);
	}
}