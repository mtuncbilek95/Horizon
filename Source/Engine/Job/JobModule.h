#pragma once

#include <Engine/Core/Subsystem.h>
#include <Engine/Job/Job.h>
#include <Engine/Job/JobWorker.h>

#include <Runtime/PAL/Sync/Atomic.h>

namespace Horizon
{
	class JobModule : public Subsystem
	{
		friend class JobWorker;

	public:
		void OnAttach(Engine* pEngine) final;
		void OnDetach() final;

		void SubmitJob(Job&& job);
		void Dispatch(JobCounter& counter, Job&& job);
		void Wait(JobCounter& counter);

	private:
		JobWorker* GetRandomVictim(JobWorker* avoidWorker);
		b8 TryRunOneJob();

	private:
		std::vector<JobWorker*> m_workers;
		Atomic<usize> m_nextWorker = 0;
	};
}