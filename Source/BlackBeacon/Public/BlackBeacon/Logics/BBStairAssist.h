#pragma once
#include "BBBeamMath.h"

namespace BlackBeacon::Logics
{
    struct FBBStairAssist
    {
        static double TargetRadius(double FeetZ)
        {
            return 178.0 - 35.0*SmoothStep(430.0,550.0,FeetZ)
                - 25.0*SmoothStep(950.0,1070.0,FeetZ);
        }
        static double TravelYaw(const BBVec3& Previous, const BBVec3& Current, bool bWalking)
        {
            if (!bWalking || (Current-Previous).Length()>40.0
                || Previous.X*Previous.X+Previous.Y*Previous.Y<6400.0) return 0.0;
            const double Cross=Previous.X*Current.Y-Previous.Y*Current.X;
            const double Dot=Previous.X*Current.X+Previous.Y*Current.Y;
            return std::atan2(Cross,Dot)*180.0/3.14159265358979323846;
        }
        static BBVec3 CenteredForward(const BBVec3& Forward, const BBVec3& Position,
            double FeetZ, double ForwardInput, double StrafeInput)
        {
            const double Radius=std::hypot(Position.X,Position.Y);
            if (Radius<80.0 || std::abs(StrafeInput)>0.1 || std::abs(ForwardInput)<0.1) return Forward;
            const BBVec3 Radial(Position.X/Radius,Position.Y/Radius,0);
            const BBVec3 Tangent(Radial.Y,-Radial.X,0);
            if (std::abs(Forward.Dot(Tangent))<0.7) return Forward;
            // A shallow correction, overridable by strafing or looking away from the stair.
            const double Correction=Clamp((TargetRadius(FeetZ)-Radius)/45.0,-0.35,0.35);
            return (Forward+Radial*(Correction*(ForwardInput>0 ? 1 : -1))).Normalized();
        }
    };
}
