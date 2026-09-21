// BLACK BEACON - power source interface.
//
// Implemented by anything that can supply power to the island's small
// grid (0.1: the generator). The power system only knows this interface.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "BBPowerSourceInterface.generated.h"

UINTERFACE(MinimalAPI)
class UBBPowerSourceInterface : public UInterface
{
	GENERATED_BODY()
};

class IBBPowerSourceInterface
{
	GENERATED_BODY()

public:
	// True when the source is producing usable power right now.
	virtual bool IsSourceActive() const = 0;

	// Current output in watts (may ramp during spin-up).
	virtual float GetCurrentWatts() const = 0;

	// Stable identifier used by the power network (and config/save data).
	virtual FName GetSourceId() const = 0;
};