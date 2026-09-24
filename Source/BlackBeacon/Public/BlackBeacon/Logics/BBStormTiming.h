#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <random>

namespace BlackBeacon::Logics
{
    inline double StormGust(double TimeSeconds)
    {
        return 0.92 + 0.16 * std::sin(TimeSeconds * 0.47) + 0.08 * std::sin(TimeSeconds * 1.13 + 0.8);
    }

    inline double ThunderDelay(double DistanceMetres)
    {
        return std::max(0.0, DistanceMetres) / 343.0;
    }

    // One event at a time: no overlapping strikes or thunder queues after leaving a storm.
    class FBBStormTiming
    {
    public:
        explicit FBBStormTiming(std::uint32_t Seed = 0xBB5701) : random(Seed) {}
        bool FlashStarted = false;
        bool ThunderDue = false;
        double DistanceMetres = 1000.0;

        void Trigger(double Distance)
        {
            DistanceMetres = std::max(343.0, Distance);
            age = 0.0;
            thunderPending = true;
            interval = std::uniform_real_distribution<double>(24.0, 52.0)(random);
        }

        void Tick(double DeltaSeconds, bool StormActive)
        {
            FlashStarted = false;
            ThunderDue = false;
            const double Dt = std::max(0.0, DeltaSeconds);
            if (!StormActive)
            {
                age = 100.0;
                thunderPending = false;
                interval = std::max(interval, 16.0);
                return;
            }
            age += Dt;
            interval -= Dt;
            if (thunderPending && age >= ThunderDelay(DistanceMetres))
            {
                ThunderDue = true;
                thunderPending = false;
            }
            if (interval <= 0.0 && !thunderPending)
            {
                Trigger(std::uniform_real_distribution<double>(700.0, 1900.0)(random));
                FlashStarted = true;
            }
        }

        double Flash() const
        {
            if (age >= 0.9) return 0.0;
            return std::clamp(age / 0.06, 0.0, 1.0) * std::exp(-age * 6.0);
        }

    private:
        std::mt19937 random;
        double age = 100.0;
        double interval = 18.0;
        bool thunderPending = false;
    };
}
