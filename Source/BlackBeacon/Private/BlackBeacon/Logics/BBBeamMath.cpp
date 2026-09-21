#include "BlackBeacon/Logics/BBBeamMath.h"

namespace BlackBeacon::Logics
{
	bool FBBBeamMath::IsPointInBeamCone(const FBBBeamQuery& Beam, const BBVec3& Point)
	{
		if (!Beam.bPowered || Beam.Intensity01 <= 0.0)
		{
			return false;
		}

		const BBVec3 ToPoint = Point - Beam.Origin;
		const double DistSq = ToPoint.LengthSq();
		if (DistSq > Beam.RangeCm * Beam.RangeCm)
		{
			return false; // beyond range
		}

		if (DistSq < 1e-6)
		{
			return true; // at the source, consider lit
		}

		const double CosAngle = ToPoint.Dot(Beam.Direction) / std::sqrt(DistSq);
		return CosAngle >= std::cos(Beam.HalfAngleRad);
	}

	double FBBBeamMath::ComputeConeIntensity(const FBBBeamQuery& Beam, const BBVec3& Point)
	{
		if (!Beam.bPowered || Beam.Intensity01 <= 0.0)
		{
			return 0.0;
		}

		const BBVec3 ToPoint = Point - Beam.Origin;
		const double DistSq = ToPoint.LengthSq();
		if (DistSq > Beam.RangeCm * Beam.RangeCm || DistSq < 1e-6)
		{
			// Out of range: nothing. At the source: treat as center-lit.
			return DistSq > Beam.RangeCm * Beam.RangeCm ? 0.0 : Beam.Intensity01;
		}

		const double CosAngle = ToPoint.Dot(Beam.Direction) / std::sqrt(DistSq);
		const double CosHalf = std::cos(Beam.HalfAngleRad);
		if (CosAngle < CosHalf)
		{
			return 0.0; // outside the cone
		}

		// Angular falloff: full intensity in the inner half of the cone,
		// smoothstep to zero at the edge. Distance is intentionally NOT
		// attenuated - range is a hard gate and reveals care about angular
		// presence, not inverse-square dimming.
		const double InnerCos = std::cos(Beam.HalfAngleRad * 0.5);
		const double Angular = SmoothStep(CosHalf, InnerCos, CosAngle);
		return Beam.Intensity01 * Angular;
	}
}