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
		TicketSlot* pSlot;

		GraphInstance(CompiledGraph&& compiled, TicketSlot* pTicket) : graph(std::move(compiled)), pending(graph.nodes.GetCount()), pSlot(pTicket)
		{
			for (usize i = 0; i < graph.nodes.GetCount(); i++)
				pending[i] = graph.nodes[i].dependencyCount;
		}
	};

	void JobSystem::ExecuteEnvelope(void* pUserData)
	{
		Envelope* pEnvelope = (Envelope*)pUserData;

		JobSystem* pSystem = pEnvelope->pSystem;
		TicketSlot* pSlot = pEnvelope->pSlot;
		GraphInstance* pGraph = pEnvelope->pGraph;
		GraphNodeId node = pEnvelope->node;

		pSlot->running.FetchAdd(1);
		pEnvelope->job.Execute();
		pSlot->running.FetchSubtract(1);

		Memory::Allocator::Delete(pEnvelope);

		if (pGraph)
			pSystem->OnNodeFinished(pGraph, node);

		if (pSystem->CompleteOne(pSlot) && pGraph)
			Memory::Allocator::Delete(pGraph);
	}

	void JobSystem::DiscardEnvelope(void* pUserData)
	{
		Envelope* pEnvelope = (Envelope*)pUserData;

		JobSystem* pSystem = pEnvelope->pSystem;
		TicketSlot* pSlot = pEnvelope->pSlot;
		GraphInstance* pGraph = pEnvelope->pGraph;

		Memory::Allocator::Delete(pEnvelope);

		if (pSystem->CompleteOne(pSlot) && pGraph)
			Memory::Allocator::Delete(pGraph);
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
			for (JobWorker* pWorker : lane.laneWorkers)
				pWorker->Stop();

			for (JobWorker* pWorker : lane.laneWorkers)
				Memory::Allocator::Delete(pWorker);
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
		TicketSlot* pSlot = AcquireSlot(lane, 1, ticket);

		if (!pSlot)
			return InvalidSubmitTicket;

		Envelope* pEnvelope = Memory::Allocator::Create<Envelope>(Memory::CurrLoc(), this, std::move(job), pSlot, nullptr, InvalidGraphNode);
		Enqueue(lane, pEnvelope);

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
		TicketSlot* pSlot = AcquireSlot(JobLane::Critical, (u32)compiledGraph.nodes.GetCount(), ticket);

		if (!pSlot)
			return InvalidSubmitTicket;

		GraphInstance* pGraph = Memory::Allocator::Create<GraphInstance>(Memory::CurrLoc(), std::move(compiledGraph), pSlot);

		List<GraphNodeId> roots;

		for (usize i = 0; i < pGraph->graph.nodes.GetCount(); i++)
		{
			if (pGraph->graph.nodes[i].dependencyCount == 0)
				roots.PushBack((GraphNodeId)i);
		}

		for (GraphNodeId root : roots)
			DispatchNode(pGraph, root);

		return ticket;
	}

	void JobSystem::SubmitDetached(JobLane lane, Job&& job)
	{
		if (!job.IsValid())
		{
			Terminal::Error(StringOps::GetName(this), "Rejected empty detached job on {} lane", LaneName(lane));
			return;
		}

		Lane& target = GetLane(lane);

		usize index = target.next.FetchAdd(1) % target.laneWorkers.GetCount();
		target.laneWorkers[index]->AddJob(std::move(job));
	}

	CompletionState JobSystem::GetTicketState(SubmitTicket ticket) const
	{
		const TicketSlot* pSlot = ResolveSlot(ticket);

		if (!pSlot)
			return CompletionState::Invalid;

		u32 generation = (u32)(ticket >> 32);

		if (pSlot->generation.Load() != generation)
			return CompletionState::Completed;

		if (pSlot->remaining.Load() == 0)
			return CompletionState::Completed;

		return pSlot->running.Load() > 0 ? CompletionState::Running : CompletionState::Pending;
	}

	b8 JobSystem::WaitTicket(SubmitTicket ticket, u64 timeoutInMs)
	{
		const TicketSlot* pSlot = ResolveSlot(ticket);

		if (!pSlot)
		{
			Terminal::Error(StringOps::GetName(this), "Cannot wait on invalid ticket {:#x}", ticket);
			return false;
		}

		JobLane helpLane = pSlot->helpLane;

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

	b8 JobSystem::CompleteOne(TicketSlot* pSlot)
	{
		if (pSlot->remaining.FetchSubtract(1) != 1)
			return false;

		ScopedLock<PAL::CriticalSection> lock(m_slotLock);
		m_freeSlots.PushBack((u32)(pSlot - m_slots));

		return true;
	}

	void JobSystem::Enqueue(JobLane lane, Envelope* pEnvelope)
	{
		Lane& target = GetLane(lane);

		usize index = target.next.FetchAdd(1) % target.laneWorkers.GetCount();
		target.laneWorkers[index]->AddJob(Job(&JobSystem::ExecuteEnvelope, &JobSystem::DiscardEnvelope, pEnvelope));
	}

	void JobSystem::DispatchNode(GraphInstance* pGraph, GraphNodeId node)
	{
		CompiledGraphNode& target = pGraph->graph.nodes[node];

		Envelope* pEnvelope = Memory::Allocator::Create<Envelope>(Memory::CurrLoc(), this, std::move(target.job), pGraph->pSlot, pGraph, node);
		Enqueue(target.lane, pEnvelope);
	}

	void JobSystem::OnNodeFinished(GraphInstance* pGraph, GraphNodeId node)
	{
		for (GraphNodeId successor : pGraph->graph.nodes[node].successors)
		{
			if (PAL::AtomicOps::FetchSubtract(&pGraph->pending[successor], 1) == 1)
				DispatchNode(pGraph, successor);
		}
	}

	JobWorker* JobSystem::GetRandomVictim(JobWorker* pAvoidWorker)
	{
		Lane& lane = GetLane(pAvoidWorker->GetLane());

		if (lane.laneWorkers.GetCount() <= 1)
			return nullptr;

		thread_local std::mt19937 range{ std::random_device{}() };
		std::uniform_int_distribution<usize> distribution(0, lane.laneWorkers.GetCount() - 2);
		usize index = distribution(range);

		if (index >= pAvoidWorker->GetWorkerIndex())
			++index;

		return lane.laneWorkers[index];
	}

	b8 JobSystem::TryRunOneJob(JobLane lane)
	{
		for (JobWorker* pWorker : GetLane(lane).laneWorkers)
		{
			Job job;
			if (pWorker->TryStealFromThis(job))
			{
				job.Execute();
				return true;
			}
		}

		return false;
	}
}