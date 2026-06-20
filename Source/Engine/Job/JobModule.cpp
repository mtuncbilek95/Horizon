#include "JobModule.h"

#include <Engine/Core/Engine.h>

#include <random>

namespace Horizon
{
	void JobModule::OnAttach(Engine* pEngine)
	{
		Submodule::OnAttach(pEngine);

		uint32_t maxWorker = std::thread::hardware_concurrency() - 1;

		for (uint32_t idx = 0; idx < maxWorker; idx++)
			m_workers.push_back(std::make_unique<JobWorker>(this, idx));
	}

	void JobModule::OnDetach()
	{
		for (usize i = 0; i < m_workers.size(); i++)
			m_workers[i]->Stop();
	}

	void JobModule::SubmitJob(Job&& job)
	{
		usize index = m_nextWorker.fetch_add(1, std::memory_order_relaxed) % m_workers.size();
		m_workers[index]->AddJob(std::move(job));
	}

	void JobModule::Dispatch(JobCounter& counter, Job&& job)
	{
		counter.remaining.fetch_add(1, std::memory_order_relaxed);

		SubmitJob([&counter, job = std::move(job)]()
			{
				job();
				counter.remaining.fetch_sub(1, std::memory_order_acq_rel);
			});
	}

	void JobModule::Wait(JobCounter& counter)
	{
		while (counter.remaining.load(std::memory_order_acquire) > 0)
		{
			if (!TryRunOneJob())
				std::this_thread::yield();
		}
	}

	JobWorker* JobModule::GetRandomVictim(JobWorker* avoidWorker)
	{
		if (m_workers.size() <= 1)
			return nullptr;

		thread_local std::mt19937 range{ std::random_device{}() };
		std::uniform_int_distribution<usize> distribution(0, m_workers.size() - 2);
		usize index = distribution(range);

		if (index >= avoidWorker->GetWorkerIndex())
			++index;

		return m_workers[index].get();
	}

	b8 JobModule::TryRunOneJob()
	{
		for (auto& worker : m_workers)
		{
			Job job;
			if (worker->TryStealFromThis(job))
			{
				job();
				return true;
			}
		}

		return false;
	}

}