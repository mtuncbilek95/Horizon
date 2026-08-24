#include "JobSystem.h"

#include <Engine/Core/Engine.h>
#include <Runtime/PAL/Hardware/Processor.h>

#include <random>

namespace Horizon::Engine
{
	JobSystem::JobSystem()
	{
		for (usize i = 0; i < PAL::Thread::HardwareConcurrency() - 1; i++)
			m_workers.PushBack(Memory::Allocator::Create<JobWorker>(Memory::CurrLoc(), this, i));

		auto cores = PAL::Processor::EnumerateCores();
		for (usize i = 0; i < m_workers.GetCount(); ++i)
		{
			const PAL::CoreInfo& core = cores[i % cores.GetCount()];

			m_workers[i]->Start();
			m_workers[i]->SetThreadAffinity(1ull << core.logicalIndex);

			Terminal::Info(StringOps::GetName(this), "Thread{} pinned to {}-Core",
				i, core.isPerformance ? "Performance" : "Efficiency");
		}

		Terminal::Debug(StringOps::GetName(this), "{} amount of thread has been initialized for multi-threading", m_workers.GetCount());
	}

	JobSystem::~JobSystem()
	{
		for (usize i = 0; i < m_workers.GetCount(); i++)
			m_workers[i]->Stop();

		for (auto* worker : m_workers)
			Memory::Allocator::Delete(worker);
	}

	void JobSystem::SubmitJob(Job&& job)
	{
		usize index = m_nextWorker.FetchAdd(1) % m_workers.GetCount();
		m_workers[index]->AddJob(std::move(job));
	}

	void JobSystem::Dispatch(JobCounter& counter, Job&& job)
	{
		counter.remaining.FetchAdd(1);

		SubmitJob([&counter, job = std::move(job)]()
			{
				job();
				counter.remaining.FetchSubtract(1);
			});
	}

	void JobSystem::Wait(JobCounter& counter)
	{
		while (counter.remaining.Load() > 0)
		{
			if (!TryRunOneJob())
				PAL::Thread::YieldCurrent();
		}
	}

	JobWorker* JobSystem::GetRandomVictim(JobWorker* avoidWorker)
	{
		if (m_workers.GetCount() <= 1)
			return nullptr;

		thread_local std::mt19937 range{ std::random_device{}() };
		std::uniform_int_distribution<usize> distribution(0, m_workers.GetCount() - 2);
		usize index = distribution(range);

		if (index >= avoidWorker->GetWorkerIndex())
			++index;

		return m_workers[index];
	}

	b8 JobSystem::TryRunOneJob()
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