// BLACK BEACON - Logics: objective graph resolver.
//
// A small, explicit dependency graph of objective nodes. In 0.1 the graph
// is a linear chain, but the resolver supports prerequisite lists so the
// chain can branch later without redesign. Pure engine-free C++.

#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace BlackBeacon::Logics
{
	struct FBBObjectiveNode
	{
		std::string Id;         // stable key, e.g. "BB_OBJ_ENTER_LIGHTHOUSE"
		std::string Text;       // player-facing objective text
		std::vector<std::string> Prereqs; // objective IDs that must be completed first
	};

	class FBBObjectiveGraph
	{
	public:
		// Replaces the graph with Nodes (declaration order is meaningful:
		// it defines which objective is "current" among eligible ones).
		void Configure(const std::vector<FBBObjectiveNode>& Nodes);

		// Marks the objective active. Only succeeds when all its prereqs
		// are completed (or the node has none). Returns true if the state
		// actually changed.
		bool ActivateObjective(const std::string& Id);

		// Completes an active, eligible objective, then auto-activates the
		// next eligible node. Returns true if completion happened.
		bool CompleteObjective(const std::string& Id);

		bool IsCompleted(const std::string& Id) const;
		bool IsActive(const std::string& Id) const;

		// The objective the player should be shown right now: the first
		// declared node that is active and not completed. Empty when the
		// graph is finished.
		std::string GetCurrentObjectiveId() const;
		bool IsFinished() const;

		size_t NumNodes() const { return Nodes.size(); }
		void Reset();
		void RestoreCompleted(const std::vector<std::string>& CompletedIds);

	private:
		bool HasPrereqs(const FBBObjectiveNode& Node) const;
		void ActivateEligibleSuccessors();

		std::vector<FBBObjectiveNode> Nodes;
		std::unordered_map<std::string, std::size_t> IdToIndex;
		std::vector<std::uint8_t> Completed;
		std::vector<std::uint8_t> Active;
	};
}
