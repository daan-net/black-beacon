#include "BlackBeacon/Power/BBPowerSystem.h"

#include "BlackBeacon/Power/BBPowerConsumerInterface.h"
#include "BlackBeacon/Power/BBPowerSourceInterface.h"

void UBBPowerSystem::RegisterSource(const TScriptInterface<IBBPowerSourceInterface>& Source)
{
	if (!Source.GetObject())
	{
		return;
	}
	for (const TScriptInterface<IBBPowerSourceInterface>& Existing : Sources)
	{
		if (Existing.GetObject() == Source.GetObject())
		{
			return; // already registered
		}
	}
	Sources.Add(Source);
	RecalculateNetwork();
}

void UBBPowerSystem::UnregisterSource(const TScriptInterface<IBBPowerSourceInterface>& Source)
{
	Sources.RemoveAll([&Source](const TScriptInterface<IBBPowerSourceInterface>& Src)
	{
		return Src.GetObject() == Source.GetObject();
	});
	RecalculateNetwork();
}

void UBBPowerSystem::RegisterConsumer(const TScriptInterface<IBBPowerConsumerInterface>& Consumer)
{
	if (!Consumer.GetObject())
	{
		return;
	}
	for (const TScriptInterface<IBBPowerConsumerInterface>& Existing : Consumers)
	{
		if (Existing.GetObject() == Consumer.GetObject())
		{
			return;
		}
	}
	Consumers.Add(Consumer);

	// Consumers learn their initial state on registration.
	RecalculateNetwork();
}

void UBBPowerSystem::UnregisterConsumer(const TScriptInterface<IBBPowerConsumerInterface>& Consumer)
{
	const FName ConsumerId = Consumer.GetInterface() ? Consumer.GetInterface()->GetConsumerId() : NAME_None;

	Consumers.RemoveAll([&Consumer](const TScriptInterface<IBBPowerConsumerInterface>& C)
	{
		return C.GetObject() == Consumer.GetObject();
	});

	if (!ConsumerId.IsNone())
	{
		SuppliedWattsByConsumer.Remove(ConsumerId);
		OnConsumerPowerChanged.Broadcast(ConsumerId, false);
	}
}

void UBBPowerSystem::ClearCache()
{
	SuppliedWattsByConsumer.Reset();
}

float UBBPowerSystem::GetTotalAvailableWatts() const
{
	float Total = 0.0f;
	for (const TScriptInterface<IBBPowerSourceInterface>& Source : Sources)
	{
		if (Source.GetInterface() && Source->IsSourceActive())
		{
			Total += Source->GetCurrentWatts();
		}
	}
	return Total;
}

void UBBPowerSystem::RecalculateNetwork()
{
	const float AvailableWatts = GetTotalAvailableWatts();
	const bool bAnyPower = AvailableWatts >= 1.0f;

	TMap<FName, float> NewSupply;
	for (const TScriptInterface<IBBPowerConsumerInterface>& Consumer : Consumers)
	{
		const FName ConsumerId = Consumer->GetConsumerId();
		const float Demand = Consumer->GetDemandWatts();
		// Consumers are fed in registration order up to the available pool.
		const float Supplied = bAnyPower ? FMath::Min(Demand, AvailableWatts) : 0.0f;
		const bool bPowered = Supplied >= Demand * 0.5f; // "good enough" threshold

		NewSupply.Add(ConsumerId, Supplied);

		// Notify on change (or first time we see this consumer).
		const float* const Previous = SuppliedWattsByConsumer.Find(ConsumerId);
		if (!Previous || (*Previous >= Demand * 0.5f) != bPowered)
		{
			Consumer->NotifyPowerState(bPowered, Supplied);
			OnConsumerPowerChanged.Broadcast(ConsumerId, bPowered);
		}
	}

	SuppliedWattsByConsumer = MoveTemp(NewSupply);
}

bool UBBPowerSystem::IsConsumerPowered(const FName& ConsumerId) const
{
	const float* const Supplied = SuppliedWattsByConsumer.Find(ConsumerId);
	return Supplied && *Supplied > 0.0f;
}

float UBBPowerSystem::GetSuppliedWatts(const FName& ConsumerId) const
{
	const float* const Supplied = SuppliedWattsByConsumer.Find(ConsumerId);
	return Supplied ? *Supplied : 0.0f;
}