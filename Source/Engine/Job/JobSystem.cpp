#include "JobSystem.h"

#include <Engine/Core/Engine.h>

#include <Runtime/PAL/Timer/Timer.h>
#include <Runtime/Containers/ScopedLock.h>

#include <algorithm>
#include <random>

namespace Horizon::Engine
{
	static const c8* LaneName(JobLane lane)
	{
		return lane == JobLane::Critical ? "Critical" : "Background";
	}

	struct JobSystem::GraphInstance
	{
		CompiledGraph graph;
		List<i64> pending;
		TicketSlot* slot;

		GraphInstance(CompiledGraph&& compiled, TicketSlot* ticket) : graph(std::move(compiled)), pending(graph.nodes.GetCount()), slot(ticket)
		{
			for (usize i = 0; i < graph.nodes.GetCount(); i++)
				pending[i] = graph.nodes[i].dependencyCount;
		}
	};

	void JobSystem::ExecuteEnvelope(void* userData)
	{
		Envelope* envelope = (Envelope*)userData;

		JobSystem* system = envelope->system;
		TicketSlot* slot = envelope->slot;
		GraphInstance* graph = envelope->graph;
		GraphNodeId node = envelope->node;

		slot->running.FetchAdd(1);
		envelope->job.Execute();
		slot->running.FetchSubtract(1);

		Memory::Allocator::Delete(envelope);

		if (graph)
			system->OnNodeFinished(graph, node);

		if (system->CompleteOne(slot) && graph)
			Memory::Allocator::Delete(graph);
	}

	void JobSystem::DiscardEnvelope(void* userData)
	{
		Envelope* envelope = (Envelope*)userData;

		JobSystem* system = envelope->system;
		TicketSlot* slot = envelope->slot;
		GraphInstance* graph = envelope->graph;

		Memory::Allocator::Delete(envelope);

		if (system->CompleteOne(slot) && graph)
			Memory::Allocator::Delete(graph);
	}

	JobSystem::JobSystem()
	{
		m_freeSlots.Reserve(MaxLiveTickets);

		for (usize i = MaxLiveTickets; i > 0; i--)
			m_freeSlots.PushBack((u32)(i - 1));

		List<PAL::CoreInfo> cores = PAL::Processor::EnumerateCores();

		if (cores.IsEmpty())
		{
			Terminal::Warn(StringOps::GetName(this), "Core enumeration failed, falling back to HardwareConcurrency");

			for (u32 i = 0; i < PAL::Thread::HardwareConcurrency(); i++)
				cores.PushBack(PAL::CoreInfo{ i, i, 0, true, true });
		}

		List<PAL::CoreInfo> performance;
		List<PAL::CoreInfo> efficiency;

		for (const PAL::CoreInfo& core : cores)
		{
			if (core.isPerformance)
				performance.PushBack(core);
			else
				efficiency.PushBack(core);
		}

		if (performance.GetCount() > 1)
			performance.PopFront();

		if (efficiency.IsEmpty())
		{
			usize backgroundCount = (std::max)(usize(1), performance.GetCount() / 4);

			while (efficiency.GetCount() < backgroundCount && performance.GetCount() > 1)
			{
				efficiency.PushBack(performance.Back());
				performance.PopBack();
			}
		}

		if (efficiency.IsEmpty())
			efficiency.PushBack(performance.Front());

		BuildLane(JobLane::Critical, performance);
		BuildLane(JobLane::Background, efficiency);
	}

	JobSystem::~JobSystem()
	{
		for (Lane& lane : m_lanes)
		{
			for (JobWorker* worker : lane.laneWorkers)
				worker->Stop();

			for (JobWorker* worker : lane.laneWorkers)
				Memory::Allocator::Delete(worker);
		}
	}

	SubmitTicket JobSystem::SubmitJob(SubmitRequest&& request)
	{
		return SubmitJob(request.lane, std::move(request.job));
	}

	SubmitTicket JobSystem::SubmitJob(JobLane lane, Job&& job)
	{
		if (!job.IsValid())
		{
			Terminal::Error(StringOps::GetName(this), "Rejected empty job on {} lane", LaneName(lane));
			return InvalidSubmitTicket;
		}

		SubmitTicket ticket = InvalidSubmitTicket;
		TicketSlot* slot = AcquireSlot(lane, 1, ticket);

		if (!slot)
			return InvalidSubmitTicket;

		Envelope* envelope = Memory::Allocator::Create<Envelope>(Memory::CurrLoc(), this, std::move(job), slot, nullptr, InvalidGraphNode);
		Enqueue(lane, envelope);

		return ticket;
	}

	SubmitTicket JobSystem::SubmitGraph(CompiledGraph&& compiledGraph)
	{
		if (!compiledGraph.IsValid())
		{
			Terminal::Error(StringOps::GetName(this), "Rejected empty or uncompiled graph");
			return InvalidSubmitTicket;
		}

		SubmitTicket ticket = InvalidSubmitTicket;
		TicketSlot* slot = AcquireSlot(JobLane::Critical, (u32)compiledGraph.nodes.GetCount(), ticket);

		if (!slot)
			return InvalidSubmitTicket;

		GraphInstance* graph = Memory::Allocator::Create<GraphInstance>(Memory::CurrLoc(), std::move(compiledGraph), slot);

		List<GraphNodeId> roots;

		for (usize i = 0; i < graph->graph.nodes.GetCount(); i++)
		{
			if (graph->graph.nodes[i].dependencyCount == 0)
				roots.PushBack((GraphNodeId)i);
		}

		for (GraphNodeId root : roots)
			DispatchNode(graph, root);

		return ticket;
	}

	CompletionState JobSystem::GetTicketState(SubmitTicket ticket) const
	{
		const TicketSlot* slot = ResolveSlot(ticket);

		if (!slot)
			return CompletionState::Invalid;

		u32 generation = (u32)(ticket >> 32);

		if (slot->generation.Load() != generation)
			return CompletionState::Completed;

		if (slot->remaining.Load() == 0)
			return CompletionState::Completed;

		return slot->running.Load() > 0 ? CompletionState::Running : CompletionState::Pending;
	}

	b8 JobSystem::WaitTicket(SubmitTicket ticket, u64 timeoutInMs)
	{
		const TicketSlot* slot = ResolveSlot(ticket);

		if (!slot)
		{
			Terminal::Error(StringOps::GetName(this), "Cannot wait on invalid ticket {:#x}", ticket);
			return false;
		}

		JobLane helpLane = slot->helpLane;

		PAL::Timer timer;
		timer.Start();

		while (GetTicketState(ticket) != CompletionState::Completed)
		{
			if (timeoutInMs != u64_max && (u64)timer.GetElapsedTimeInMs() >= timeoutInMs)
				return false;

			if (!TryRunOneJob(helpLane))
				PAL::Thread::YieldCurrent();
		}

		return true;
	}

	void JobSystem::BuildLane(JobLane lane, const List<PAL::CoreInfo>& cores)
	{
		Lane& target = GetLane(lane);

		for (usize i = 0; i < cores.GetCount(); i++)
			target.laneWorkers.PushBack(Memory::Allocator::Create<JobWorker>(Memory::CurrLoc(), this, lane, i));

		for (usize i = 0; i < cores.GetCount(); i++)
		{
			target.laneWorkers[i]->Start();
			target.laneWorkers[i]->SetThreadAffinity(1ull << cores[i].logicalIndex);

			Terminal::Info(StringOps::GetName(this), "{} worker {} pinned to logical core {} ({})",
				LaneName(lane), i, cores[i].logicalIndex, cores[i].isPerformance ? "Performance" : "Efficiency");
		}

		Terminal::Debug(StringOps::GetName(this), "{} lane initialized with {} workers", LaneName(lane), target.laneWorkers.GetCount());
	}

	JobSystem::TicketSlot* JobSystem::AcquireSlot(JobLane helpLane, u32 jobCount, SubmitTicket& outTicket)
	{
		u32 index = kInvalid32;

		{
			ScopedLock<PAL::CriticalSection> lock(m_slotLock);

			if (!m_freeSlots.IsEmpty())
			{
				index = m_freeSlots.Back();
				m_freeSlots.PopBack();
			}
		}

		if (index == kInvalid32)
		{
			Terminal::Error(StringOps::GetName(this), "No free ticket slot, {} tickets are already live", MaxLiveTickets);
			outTicket = InvalidSubmitTicket;
			return nullptr;
		}

		TicketSlot& slot = m_slots[index];

		slot.helpLane = helpLane;
		slot.running.Store(0);
		slot.remaining.Store(jobCount);

		u32 generation = slot.generation.FetchAdd(1) + 1;

		outTicket = (SubmitTicket(generation) << 32) | index;
		return &slot;
	}

	const JobSystem::TicketSlot* JobSystem::ResolveSlot(SubmitTicket ticket) const
	{
		if (ticket == InvalidSubmitTicket)
			return nullptr;

		u32 index = (u32)(ticket & 0xFFFFFFFFull);

		if (index >= MaxLiveTickets)
			return nullptr;

		return &m_slots[index];
	}

	b8 JobSystem::CompleteOne(TicketSlot* slot)
	{
		if (slot->remaining.FetchSubtract(1) != 1)
			return false;

		ScopedLock<PAL::CriticalSection> lock(m_slotLock);
		m_freeSlots.PushBack((u32)(slot - m_slots));

		return true;
	}

	void JobSystem::Enqueue(JobLane lane, Envelope* envelope)
	{
		Lane& target = GetLane(lane);

		usize index = target.next.FetchAdd(1) % target.laneWorkers.GetCount();
		target.laneWorkers[index]->AddJob(Job(&JobSystem::ExecuteEnvelope, &JobSystem::DiscardEnvelope, envelope));
	}

	void JobSystem::DispatchNode(GraphInstance* graph, GraphNodeId node)
	{
		CompiledGraphNode& target = graph->graph.nodes[node];

		Envelope* envelope = Memory::Allocator::Create<Envelope>(Memory::CurrLoc(), this, std::move(target.job), graph->slot, graph, node);
		Enqueue(target.lane, envelope);
	}

	void JobSystem::OnNodeFinished(GraphInstance* graph, GraphNodeId node)
	{
		for (GraphNodeId successor : graph->graph.nodes[node].successors)
		{
			if (PAL::AtomicOps::FetchSubtract(&graph->pending[successor], 1) == 1)
				DispatchNode(graph, successor);
		}
	}

	JobWorker* JobSystem::GetRandomVictim(JobWorker* avoidWorker)
	{
		Lane& lane = GetLane(avoidWorker->GetLane());

		if (lane.laneWorkers.GetCount() <= 1)
			return nullptr;

		thread_local std::mt19937 range{ std::random_device{}() };
		std::uniform_int_distribution<usize> distribution(0, lane.laneWorkers.GetCount() - 2);
		usize index = distribution(range);

		if (index >= avoidWorker->GetWorkerIndex())
			++index;

		return lane.laneWorkers[index];
	}

	b8 JobSystem::TryRunOneJob(JobLane lane)
	{
		for (JobWorker* worker : GetLane(lane).laneWorkers)
		{
			Job job;
			if (worker->TryStealFromThis(job))
			{
				job.Execute();
				return true;
			}
		}

		return false;
	}
}