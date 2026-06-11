#include "JobModule.h"

#include <Engine/Engine/Engine.h>

namespace Horizon
{
	void JobModule::OnAttach(Engine& engine)
	{
		IModule::OnAttach(engine);

		const u32 hardwareThreads = std::thread::hardware_concurrency();
		m_workerCount = hardwareThreads > 1 ? hardwareThreads - 1 : 1;

		m_pool.Start(m_workerCount);
	}

	void JobModule::OnDetach()
	{
		m_pool.Stop();
	}

	void JobModule::Submit(JobFunc job)
	{
		m_pool.Push(std::move(job));
	}

	void JobModule::Submit(JobCounter& counter, JobFunc job)
	{
		counter.remaining.fetch_add(1, std::memory_order_relaxed);

		m_pool.Push([&counter, job = std::move(job)]()
			{
				job();
				counter.remaining.fetch_sub(1, std::memory_order_release);
			});
	}

	void JobModule::Dispatch(JobCounter& counter, u32 count, u32 groupSize, JobRangeFunc fn)
	{
		if (count == 0)
			return;

		groupSize = groupSize == 0 ? 1 : groupSize;
		const u32 groupCount = (count + groupSize - 1) / groupSize;

		for (u32 g = 0; g < groupCount; g++)
		{
			const u32 begin = g * groupSize;
			const u32 end = begin + groupSize < count ? begin + groupSize : count;

			Submit(counter, [fn, begin, end]()
				{
					fn(begin, end);
				});
		}
	}

	void JobModule::Wait(JobCounter& counter)
	{
		while (!counter.IsDone())
		{
			if (!m_pool.TryExecuteOne())
				std::this_thread::yield();
		}
	}
}
