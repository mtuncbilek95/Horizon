#pragma once

#include <Engine/Job/Job.h>

#include <Runtime/PAL/Sync/Thread.h>
#include <Runtime/PAL/Sync/CriticalSection.h>
#include <Runtime/PAL/Sync/Semaphore.h>
#include <Runtime/PAL/Sync/Atomic.h>

#include <queue>
#include <shared_mutex>
#include <condition_variable>
#include <atomic>

namespace Horizon
{
	class JobModule;

	class JobWorker
	{
		static void ThreadEntryPoint(void* userData);
	public:
		JobWorker(JobModule* pModule, usize index);
		~JobWorker() = default;

		void Run();
		void Stop();

		void AddJob(Job&& job);

		b8 TryStealFromThis(Job& out);
		usize GetWorkerIndex() const { return m_index; }

		void SetThreadAffinity(u64 mask);

	private:
		b8 TryPopJob(Job& out);

	private:
		std::deque<Job> m_jobs;
		JobModule* m_owner;

		Thread m_worker;
		CriticalSection m_mutex;
		Semaphore m_signal;

		usize m_index;

		Atomic<b8> m_working = false;
	};
}