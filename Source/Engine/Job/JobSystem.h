#pragma once

#include <Engine/Core/Context.h>
#include <Engine/Job/Job.h>
#include <Engine/Job/JobWorker.h>

#include <Runtime/PAL/Sync/Atomic.h>
#include <Runtime/Containers/List.h>

namespace Horizon::Engine
{
	class H_EXPORT JobSystem final
	{
		friend class JobWorker;
	public:
		JobSystem();
		~JobSystem();

		void SubmitJob(Job&& job);
		void Dispatch(JobCounter& counter, Job&& job);
		void Wait(JobCounter& counter);

	private:
		JobWorker* GetRandomVictim(JobWorker* avoidWorker);
		b8 TryRunOneJob();

	private:
		List<JobWorker*> m_workers;
		PAL::Atomic<usize> m_nextWorker = 0;
	};
}