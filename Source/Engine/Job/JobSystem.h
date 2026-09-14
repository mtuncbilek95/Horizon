#pragma once

#include <Engine/Core/Context.h>
#include <Engine/Job/Job.h>
#include <Engine/Job/JobLane.h>
#include <Engine/Job/JobTicket.h>
#include <Engine/Job/JobGraph.h>
#include <Engine/Job/Lane.h>
#include <Engine/Job/JobWorker.h>

#include <Runtime/PAL/Hardware/Processor.h>
#include <Runtime/PAL/Sync/Atomic.h>
#include <Runtime/PAL/Sync/CriticalSection.h>
#include <Runtime/Containers/List.h>

namespace Horizon::Engine
{
	class H_EXPORT JobSystem final
	{
		friend class JobWorker;

		struct TicketSlot
		{
			PAL::Atomic<u32> generation = 0;
			PAL::Atomic<u32> remaining = 0;
			PAL::Atomic<u32> running = 0;
			JobLane helpLane = JobLane::Critical;
		};

		struct GraphInstance;

		struct Envelope
		{
			JobSystem* system;
			Job job;
			TicketSlot* slot;
			GraphInstance* graph;
			GraphNodeId node;

			Envelope(JobSystem* owner, Job&& work, TicketSlot* ticket, GraphInstance* instance, GraphNodeId nodeId)
				: system(owner), job(std::move(work)), slot(ticket), graph(instance), node(nodeId)
			{
			}
		};

		static void ExecuteEnvelope(void* userData);
		static void DiscardEnvelope(void* userData);

	public:
		static constexpr usize MaxLiveTickets = 4096;

		JobSystem();
		~JobSystem();

		SubmitTicket SubmitJob(SubmitRequest&& request);
		SubmitTicket SubmitJob(JobLane lane, Job&& job);
		SubmitTicket SubmitGraph(CompiledGraph&& compiledGraph);

		CompletionState GetTicketState(SubmitTicket ticket) const;
		b8 WaitTicket(SubmitTicket ticket, u64 timeoutInMs = u64_max);

		usize GetWorkerCount(JobLane lane) const { return GetLane(lane).laneWorkers.GetCount(); }

	private:
		void BuildLane(JobLane lane, const List<PAL::CoreInfo>& cores);

		Lane& GetLane(JobLane lane) { return m_lanes[(usize)lane]; }
		const Lane& GetLane(JobLane lane) const { return m_lanes[(usize)lane]; }

		TicketSlot* AcquireSlot(JobLane helpLane, u32 jobCount, SubmitTicket& outTicket);
		const TicketSlot* ResolveSlot(SubmitTicket ticket) const;
		b8 CompleteOne(TicketSlot* slot);

		void Enqueue(JobLane lane, Envelope* envelope);
		void DispatchNode(GraphInstance* graph, GraphNodeId node);
		void OnNodeFinished(GraphInstance* graph, GraphNodeId node);

		JobWorker* GetRandomVictim(JobWorker* avoidWorker);
		b8 TryRunOneJob(JobLane lane);

	private:
		Lane m_lanes[2];

		TicketSlot m_slots[MaxLiveTickets];
		List<u32> m_freeSlots;
		PAL::CriticalSection m_slotLock;
	};
}