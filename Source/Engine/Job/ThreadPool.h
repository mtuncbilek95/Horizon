#pragma once

#include <Engine/Job/Job.h>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <vector>
#include <atomic>

namespace Horizon
{
	class ThreadPool final
	{
	public:
		void Start(u32 workerCount);
		void Stop();

		void Push(JobFunc&& job);
		b8 TryExecuteOne();

		static u32 LocalWorkerIndex();

	private:
		void WorkerLoop(u32 selfIndex);

		std::deque<JobFunc> m_queue;
		std::mutex m_mutex;
		std::condition_variable m_signal;
		std::vector<std::thread> m_workers;
		std::atomic<b8> m_quit = false;
	};

}