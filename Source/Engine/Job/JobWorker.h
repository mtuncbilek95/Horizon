#pragma once

#include <Engine/Job/Job.h>

#include <Runtime/Containers/WorkStealingDeque.h>
#include <Runtime/PAL/Sync/Thread.h>
#include <Runtime/PAL/Sync/CriticalSection.h>
#include <Runtime/PAL/Sync/Futex.h>
#include <Runtime/PAL/Sync/Atomic.h>

namespace Horizon
{
	class JobContext;

	class H_EXPORT JobWorker
	{
		static void ThreadEntryPoint(void* userData);

		struct JobNode
		{
			Job job;
			JobNode* next;
		};

	public:
		JobWorker(JobContext* pContext, usize index);
		~JobWorker();

		void Run();
		void Stop();

		void AddJob(Job&& job);

		b8 TryStealFromThis(Job& out);
		usize GetWorkerIndex() const { return m_index; }

		void SetThreadAffinity(u64 mask);

	private:
		void DrainInbox();
		b8 TryPopJob(Job& out);

	private:
		JobContext* m_owner;
		usize m_index;

		WorkStealingDeque<JobNode*> m_deque;
		PAL::Atomic<JobNode*> m_inbox;

		PAL::Atomic<i64> m_signal;
		PAL::Atomic<b8> m_working;

		PAL::Thread m_worker;
	};
}