#pragma once

#include <Engine/Engine/IModule.h>
#include <Engine/Job/Job.h>
#include <Engine/Job/JobWorker.h>

namespace Horizon
{
	class JobModule : public IModule
	{
		friend class JobWorker;
	public:
		void OnAttach(Engine& engine) final;
		void OnDetach() final;

		void SubmitJob(Job&& job);
		void Dispatch(JobCounter& counter, Job&& job);
		void Wait(JobCounter& counter);

	private:
		JobWorker* GetRandomVictim(JobWorker* avoidWorker);
		b8 TryRunOneJob();

	private:
		std::vector<std::unique_ptr<JobWorker>> m_workers;
		std::atomic<usize> m_nextWorker = 0;
	};
}