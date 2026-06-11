#include "JobModule.h"

#include <Engine/Engine/Engine.h>

#include <random>

namespace Horizon
{
	void JobModule::OnAttach(Engine& engine)
	{
		IModule::OnAttach(engine);

		uint32_t maxWorker = std::thread::hardware_concurrency() - 1;

		for (uint32_t idx = 0; idx < maxWorker; idx++)
			m_workers.push_back(std::make_unique<JobWorker>(this, idx));
	}

	void JobModule::OnDetach()
	{
	}

	void JobModule::SubmitJob(Job&& job)
	{
		usize index = m_nextWorker.fetch_add(1, std::memory_order_relaxed) % m_workers.size();
		m_workers[index]->AddJob(std::move(job));
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
}
