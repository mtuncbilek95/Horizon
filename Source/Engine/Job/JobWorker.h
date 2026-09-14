#pragma once

#include <Engine/Job/Job.h>
#include <Engine/Job/JobLane.h>

#include <Runtime/Containers/WorkStealingDeque.h>
#include <Runtime/PAL/Sync/Thread.h>
#include <Runtime/PAL/Sync/Futex.h>
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

			JobNode(Job&& work) : job(std::move(work)), next(nullptr)
			{
			}
		};

	public:
		JobWorker(JobSystem* pContext, JobLane lane, usize index);
		~JobWorker();

		void Start();
		void Run();
		void Stop();

		void AddJob(Job&& job);

		b8 TryStealFromThis(Job& out);

		JobLane GetLane() const { return m_lane; }
		usize GetWorkerIndex() const { return m_index; }

		void SetThreadAffinity(u64 mask);

	private:
		void DrainInbox();
		b8 TryPopJob(Job& out);

	private:
		JobSystem* m_owner;
		JobLane m_lane;
		usize m_index;

		WorkStealingDeque<JobNode*> m_deque;
		PAL::Atomic<JobNode*> m_inbox;

		PAL::Atomic<i64> m_signal;
		PAL::Atomic<b8> m_working;

		PAL::Thread m_worker;
	};
}