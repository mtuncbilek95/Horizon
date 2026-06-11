#pragma once

#include <Engine/Job/Job.h>

#include <queue>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <atomic>

namespace Horizon
{
	class JobModule;

	class JobWorker
	{
	public:
		JobWorker(JobModule* pModule, usize index);
		~JobWorker() = default;

		void Run();
		void Stop();

		void AddJob(Job&& job);

		b8 TryStealFromThis(Job& out);
		usize GetWorkerIndex() const { return m_index; }

	private:
		b8 TryPopJob(Job& out);

	private:
		std::deque<Job> m_jobs;
		std::thread m_worker;
		std::mutex m_mutex;
		std::condition_variable m_condition;
		std::atomic<b8> m_working = false;

		usize m_index;

		JobModule* m_owner;
	};
}