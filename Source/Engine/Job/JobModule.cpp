#include "JobModule.h"

#include <Engine/Core/Engine.h>

#include <random>

namespace Horizon
{
	void JobModule::OnAttach(Engine* pEngine)
	{
		Subsystem::OnAttach(pEngine);

		for (usize i = 0; i < Thread::HardwareConcurrency() - 1; i++)
			m_workers.push_back(Allocator::Create<JobWorker>(CurrLoc(), this, i));

		auto cores = Thread::EnumerateCores();
		for (usize i = 0; i < m_workers.size(); ++i)
		{
			const CoreInfo& core = cores[i % cores.size()];
			m_workers[i]->SetThreadAffinity(1ull << core.logicalIndex);

			Terminal::Info("JobModule", "Thread{} pinned to {}-Core",
				i, core.isPerformance ? "Performance" : "Efficiency");
		}
	}

	void JobModule::OnDetach()
	{
		for (usize i = 0; i < m_workers.size(); i++)
			m_workers[i]->Stop();

		for (auto* worker : m_workers)
			Allocator::Delete(worker);
	}

	void JobModule::SubmitJob(Job&& job)
	{
		usize index = m_nextWorker.FetchAdd(1) % m_workers.size();
		m_workers[index]->AddJob(std::move(job));
	}

	void JobModule::Dispatch(JobCounter& counter, Job&& job)
	{
		counter.remaining.FetchAdd(1);

		SubmitJob([&counter, job = std::move(job)]()
			{
				job();
				counter.remaining.FetchSubtract(1);
			});
	}

	void JobModule::Wait(JobCounter& counter)
	{
		while (counter.remaining.Load() > 0)
		{
			if (!TryRunOneJob())
				Thread::YieldCurrent();
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

		return m_workers[index];
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