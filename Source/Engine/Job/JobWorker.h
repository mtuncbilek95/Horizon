#pragma once

#include <Engine/Job/Job.h>
#include <Engine/Job/JobLane.h>

#include <Runtime/Containers/WorkStealingDeque.h>
#include <Runtime/PAL/Sync/Thread.h>
#include <Runtime/PAL/Sync/Atomic.h>

namespace Horizon::Engine
{
	class JobSystem;

	class H_EXPORT JobWorker
	{
		static void ThreadEntryPoint(void* userData);

		struct JobNode
		{
			Job job;
			JobNode* next;
		};

	public:
		JobWorker(JobSystem* pSystem, u32 index, JobLane lane);
		~JobWorker();

		void Configure(u64 affinity, PAL::ThreadPriority priority, b8 ecoQoS);
		void Start();
		void Run();
		void RequestStop();
		void Join();

		void AddJob(Job&& job);
		void PushLocal(Job&& job);

		b8 TryStealFromThis(Job& out);

		JobLane GetLane() const { return m_lane; }
		u32 GetWorkerIndex() const { return m_index; }
		u64 GetAffinity() const { return m_affinity; }

	private:
		void DrainInbox();
		b8 TryPopJob(Job& out);

	private:
		JobSystem* m_owner;
		u32 m_index;
		JobLane m_lane;

		u64 m_affinity;
		PAL::ThreadPriority m_priority;
		b8 m_ecoQoS;

		WorkStealingDeque<JobNode*> m_deque;
		PAL::Atomic<JobNode*> m_inbox;
		PAL::Atomic<b8> m_working;

		PAL::Thread m_worker;
	};
}