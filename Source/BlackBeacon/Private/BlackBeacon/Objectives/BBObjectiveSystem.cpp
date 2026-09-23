#include "BlackBeacon/Objectives/BBObjectiveSystem.h"

#include "Containers/StringConv.h"

void UBBObjectiveSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	BuildGraphFromConfig();
	PublishCurrent();
}

void UBBObjectiveSystem::BuildGraphFromConfig()
{
	std::vector<BlackBeacon::Logics::FBBObjectiveNode> Nodes;
	Nodes.reserve(ObjectiveChain.Num());

	for (const FBBObjectiveNodeConfig& NodeConfig : ObjectiveChain)
	{
		BlackBeacon::Logics::FBBObjectiveNode Node;
		const FTCHARToUTF8 IdConverter(*NodeConfig.Id);
		const FTCHARToUTF8 TextConverter(*NodeConfig.Text);
		Node.Id.assign(IdConverter.Get(), IdConverter.Length());
		Node.Text.assign(TextConverter.Get(), TextConverter.Length());
		for (const FString& Prereq : NodeConfig.Prereqs)
		{
			const FTCHARToUTF8 PrereqConverter(*Prereq);
			Node.Prereqs.emplace_back(PrereqConverter.Get(), PrereqConverter.Length());
		}
		Nodes.push_back(MoveTemp(Node));
	}

	Graph.Configure(Nodes);
	bGraphBuilt = true;
}

bool UBBObjectiveSystem::ActivateObjective(const FString& ObjectiveId)
{
	if (!bGraphBuilt)
	{
		return false;
	}
	const FTCHARToUTF8 Converter(*ObjectiveId);
	const bool bChanged = Graph.ActivateObjective(std::string(Converter.Get(), Converter.Length()));
	if (bChanged)
	{
		// The "current" objective may have changed if the new node precedes
		// the previously shown one; refresh the HUD contract.
		PublishCurrent();
	}
	return bChanged;
}

bool UBBObjectiveSystem::CompleteObjective(const FString& ObjectiveId)
{
	if (!bGraphBuilt)
	{
		return false;
	}

	const FTCHARToUTF8 Converter(*ObjectiveId);
	const std::string IdUtf8(Converter.Get(), Converter.Length());

	const bool bWasActive = Graph.IsActive(IdUtf8);
	const bool bCompleted = Graph.CompleteObjective(IdUtf8);
	if (!bCompleted)
	{
		return false;
	}

	OnObjectiveCompleted.Broadcast(ObjectiveId, bWasActive);

	const bool bNowFinished = Graph.IsFinished();
	if (bNowFinished != bLastFinished)
	{
		bLastFinished = bNowFinished;
	}
	PublishCurrent();
	return true;
}

bool UBBObjectiveSystem::IsCompleted(const FString& ObjectiveId) const
{
	const FTCHARToUTF8 Converter(*ObjectiveId);
	return bGraphBuilt && Graph.IsCompleted(std::string(Converter.Get(), Converter.Length()));
}

bool UBBObjectiveSystem::IsActive(const FString& ObjectiveId) const
{
	const FTCHARToUTF8 Converter(*ObjectiveId);
	return bGraphBuilt && Graph.IsActive(std::string(Converter.Get(), Converter.Length()));
}

bool UBBObjectiveSystem::IsFinished() const
{
	return bGraphBuilt && Graph.IsFinished();
}

FString UBBObjectiveSystem::GetCurrentObjectiveId() const
{
	if (!bGraphBuilt)
	{
		return FString();
	}
	const std::string Id = Graph.GetCurrentObjectiveId();
	const FUTF8ToTCHAR Converter(Id.data(), static_cast<int32>(Id.size()));
	return FString(Converter.Length(), Converter.Get());
}

FString UBBObjectiveSystem::GetCurrentObjectiveText() const
{
	const FString CurrentId = GetCurrentObjectiveId();
	if (CurrentId.IsEmpty())
	{
		return FString();
	}

	for (const FBBObjectiveNodeConfig& Node : ObjectiveChain)
	{
		if (Node.Id == CurrentId)
		{
			return Node.Text;
		}
	}
	return FString();
}

void UBBObjectiveSystem::PublishCurrent()
{
	const FString CurrentId = GetCurrentObjectiveId();
	const FString CurrentText = GetCurrentObjectiveText();
	OnCurrentObjectiveChanged.Broadcast(CurrentId, CurrentText);
}

TArray<FString> UBBObjectiveSystem::GetCompletedObjectives() const
{
	TArray<FString> Completed;
	for (const FBBObjectiveNodeConfig& Node : ObjectiveChain)
	{
		if (IsCompleted(Node.Id))
		{
			Completed.Add(Node.Id);
		}
	}
	return Completed;
}

void UBBObjectiveSystem::RestoreCompletedObjectives(const TArray<FString>& Objectives)
{
	for (const FString& Id : Objectives)
	{
		CompleteObjective(Id);
	}
}
