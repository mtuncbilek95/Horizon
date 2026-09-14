#pragma once

#include <Engine/Job/Job.h>
#include <Engine/Job/JobLane.h>

#include <Runtime/Containers/List.h>

namespace Horizon::Engine
{
	using GraphNodeId = u32;
	static constexpr GraphNodeId InvalidGraphNode = kInvalid32;

	struct H_EXPORT CompiledGraphNode
	{
		JobLane lane = JobLane::Critical;
		Job job;
		u32 dependencyCount = 0;
		List<GraphNodeId> successors;

		CompiledGraphNode() = default;

		CompiledGraphNode(const CompiledGraphNode&) = delete;
		CompiledGraphNode& operator=(const CompiledGraphNode&) = delete;

		CompiledGraphNode(CompiledGraphNode&&) noexcept = default;
		CompiledGraphNode& operator=(CompiledGraphNode&&) noexcept = default;
	};

	struct H_EXPORT CompiledGraph
	{
		List<CompiledGraphNode> nodes;

		CompiledGraph() = default;

		CompiledGraph(const CompiledGraph&) = delete;
		CompiledGraph& operator=(const CompiledGraph&) = delete;

		CompiledGraph(CompiledGraph&&) noexcept = default;
		CompiledGraph& operator=(CompiledGraph&&) noexcept = default;

		b8 IsValid() const { return !nodes.IsEmpty(); }
	};

	class H_EXPORT JobGraph final
	{
	public:
		JobGraph() = default;

		JobGraph(const JobGraph&) = delete;
		JobGraph& operator=(const JobGraph&) = delete;

		JobGraph(JobGraph&&) noexcept = default;
		JobGraph& operator=(JobGraph&&) noexcept = default;

		GraphNodeId AddNode(JobLane lane, Job&& job);
		GraphNodeId AddAfter(GraphNodeId before, JobLane lane, Job&& job);
		b8 AddDependency(GraphNodeId before, GraphNodeId after);

		CompiledGraph Compile();

		usize GetNodeCount() const { return m_nodes.GetCount(); }

	private:
		List<CompiledGraphNode> m_nodes;
	};
}