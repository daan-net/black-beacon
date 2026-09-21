// BLACK BEACON - power consumer interface.
//
// Implemented by anything that draws power (0.1: the lighthouse).
// The power system grants/revokes power through NotifyPowerState and
// consumers react (e.g. the beam turns on when powered).

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "BBPowerConsumerInterface.generated.h"

UINTERFACE(MinimalAPI)
class UBBPowerConsumerInterface : public UInterface
{
	GENERATED_BODY()
};

class IBBPowerConsumerInterface
{
	GENERATED_BODY()

public:
	virtual FName GetConsumerId() const = 0;

	// Watts this consumer needs to run at nominal state.
	virtual float GetDemandWatts() const = 0;

	// Called by the power system whenever the network recalculates and
	// this consumer's state changed (or on first registration).
	virtual void NotifyPowerState(bool bPoweredNow, float SuppliedWatts) = 0;
};