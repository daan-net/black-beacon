#include "BlackBeacon/Logics/BBObjectiveGraph.h"

namespace BlackBeacon::Logics
{
	void FBBObjectiveGraph::Configure(const std::vector<FBBObjectiveNode>& InNodes)
	{
		Nodes = InNodes;
		IdToIndex.clear();
		Completed.assign(Nodes.size(), 0);
		Active.assign(Nodes.size(), 0);

		for (std::size_t I = 0; I < Nodes.size(); ++I)
		{
			IdToIndex.emplace(Nodes[I].Id, I);
		}

		// The declared chain is unlocked from the front: any node whose
		// prereqs are satisfied (or that has none) becomes active.
		ActivateEligibleSuccessors();
	}

	bool FBBObjectiveGraph::HasPrereqs(const FBBObjectiveNode& Node) const
	{
		for (const std::string& Prereq : Node.Prereqs)
		{
			const auto Found = IdToIndex.find(Prereq);
			if (Found == IdToIndex.end() || !Completed[Found->second])
			{
				return false;
			}
		}
		return true;
	}

	void FBBObjectiveGraph::ActivateEligibleSuccessors()
	{
		for (std::size_t I = 0; I < Nodes.size(); ++I)
		{
			if (!Active[I] && !Completed[I] && HasPrereqs(Nodes[I]))
			{
				Active[I] = 1;
			}
		}
	}

	bool FBBObjectiveGraph::ActivateObjective(const std::string& Id)
	{
		const auto Found = IdToIndex.find(Id);
		if (Found == IdToIndex.end())
		{
			return false;
		}
		const std::size_t Index = Found->second;
		if (Active[Index] || Completed[Index])
		{
			return false; // already active/completed - nothing to do
		}
		if (!HasPrereqs(Nodes[Index]))
		{
			return false; // locked behind uncompleted prerequisites
		}
		Active[Index] = 1;
		return true;
	}

	bool FBBObjectiveGraph::CompleteObjective(const std::string& Id)
	{
		const auto Found = IdToIndex.find(Id);
		if (Found == IdToIndex.end())
		{
			return false;
		}
		const std::size_t Index = Found->second;
		if (Completed[Index] || !Active[Index] || !HasPrereqs(Nodes[Index]))
		{
			return false;
		}
		Completed[Index] = 1;
		ActivateEligibleSuccessors();
		return true;
	}

	bool FBBObjectiveGraph::IsCompleted(const std::string& Id) const
	{
		const auto Found = IdToIndex.find(Id);
		return Found != IdToIndex.end() && Completed[Found->second];
	}

	bool FBBObjectiveGraph::IsActive(const std::string& Id) const
	{
		const auto Found = IdToIndex.find(Id);
		return Found != IdToIndex.end() && Active[Found->second] && !Completed[Found->second];
	}

	std::string FBBObjectiveGraph::GetCurrentObjectiveId() const
	{
		for (std::size_t I = 0; I < Nodes.size(); ++I)
		{
			if (Active[I] && !Completed[I])
			{
				return Nodes[I].Id;
			}
		}
		return std::string();
	}

	bool FBBObjectiveGraph::IsFinished() const
	{
		for (std::size_t I = 0; I < Nodes.size(); ++I)
		{
			if (!Completed[I])
			{
				return false;
			}
		}
		return true;
	}

	void FBBObjectiveGraph::Reset()
	{
		Completed.assign(Nodes.size(), 0);
		Active.assign(Nodes.size(), 0);
		ActivateEligibleSuccessors();
	}

	void FBBObjectiveGraph::RestoreCompleted(const std::vector<std::string>& CompletedIds)
	{
		Reset();
		for (const FBBObjectiveNode& Node : Nodes)
		{
			for (const std::string& CompletedId : CompletedIds)
			{
				if (Node.Id == CompletedId)
				{
					CompleteObjective(Node.Id);
					break;
				}
			}
		}
	}
}
