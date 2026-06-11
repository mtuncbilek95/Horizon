#include "ThreadPool.h"

namespace Horizon
{
	static thread_local u32 tWorkerIndex = 0;

	u32 ThreadPool::LocalWorkerIndex()
	{
		return tWorkerIndex;
	}

	void ThreadPool::Start(u32 workerCount)
	{
		m_quit.store(false, std::memory_order_relaxed);

		m_workers.reserve(workerCount);
		for (u32 i = 0; i < workerCount; i++)
			m_workers.emplace_back(&ThreadPool::WorkerLoop, this, i + 1);
	}

	void ThreadPool::Stop()
	{
		{
			std::lock_guard lock(m_mutex);
			m_quit.store(true, std::memory_order_relaxed);
		}
		m_signal.notify_all();

		for (std::thread& worker : m_workers)
		{
			if (worker.joinable())
				worker.join();
		}

		m_workers.clear();
		m_queue.clear();
	}

	void ThreadPool::Push(JobFunc&& job)
	{
		{
			std::lock_guard lock(m_mutex);
			m_queue.push_back(std::move(job));
		}
		m_signal.notify_one();
	}

	b8 ThreadPool::TryExecuteOne()
	{
		JobFunc job;
		{
			std::lock_guard lock(m_mutex);
			if (m_queue.empty())
				return false;

			job = std::move(m_queue.front());
			m_queue.pop_front();
		}

		job();
		return true;
	}

	void ThreadPool::WorkerLoop(u32 selfIndex)
	{
		tWorkerIndex = selfIndex;

		constexpr u32 SpinLimit = 256;
		u32 spins = 0;

		while (!m_quit.load(std::memory_order_relaxed))
		{
			if (TryExecuteOne())
			{
				spins = 0;
				continue;
			}

			if (++spins < SpinLimit)
			{
				std::this_thread::yield();
				continue;
			}

			std::unique_lock lock(m_mutex);
			m_signal.wait(lock, [this] { return m_quit.load(std::memory_order_relaxed) || !m_queue.empty(); });
			spins = 0;
		}
	}
}
