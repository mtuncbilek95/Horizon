#include "JobSystem.h"

namespace Horizon
{
	EngineReport JobSystem::OnInitialize()
	{
		u32 count = std::thread::hardware_concurrency() - 1;
		m_workers.resize(count);

		for (u32 i = 0; i < count; i++)
			m_workers[i] = std::thread(&JobSystem::WorkerLoop, this, i);

		return EngineReport();
	}

	void JobSystem::WorkerLoop(u32 workerIndex)
	{
		while (true)
		{
			Job job;
			{
				std::unique_lock<std::mutex> lock(m_mutex);
				m_condition.wait(lock, [this]() {
					return !m_jobQueue.empty() || m_shutdown;
					});

				if (m_shutdown && m_jobQueue.empty())
					return;

				job = std::move(m_jobQueue.front());
				m_jobQueue.pop();
			}

			job();
		}
	}

	void JobSystem::Submit(std::function<void()> job)
	{
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_jobQueue.push(std::move(job));
		}
		m_condition.notify_one();
	}

	u32 JobSystem::WorkerCount() const
	{
		return static_cast<u32>(m_workers.size());
	}

	void JobSystem::OnSync()
	{}

	void JobSystem::OnFinalize()
	{
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_shutdown = true;
		}
		m_condition.notify_all();

		for (auto& worker : m_workers)
			if (worker.joinable())
				worker.join();
	}
}