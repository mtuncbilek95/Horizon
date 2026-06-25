#pragma once

#include <Engine/Core/Subsystem.h>
#include <Engine/Job/Job.h>
#include <Engine/Job/JobWorker.h>

#include <Runtime/PAL/Sync/Atomic.h>

namespace Horizon
{
	/*
	 * First module that always needs to be run, so every other modules can 
	 * use this whenever they want.
	 */
	class JobSubsystem : public Subsystem
	{
		friend class JobWorker;
	public:
		EngineReport OnAttach(Engine* pEngine) final;
		void OnDetach() final;

		void SubmitJob(Job&& job);
		void Dispatch(JobCounter& counter, Job&& job);
		void Wait(JobCounter& counter);

		void GetInitializeOrder(OrderRules& rules) const final;
		void GetExecutionOrder(OrderRules& rules) const final;

	private:
		JobWorker* GetRandomVictim(JobWorker* avoidWorker);
		b8 TryRunOneJob();

	private:
		std::vector<JobWorker*> m_workers;
		PAL::Atomic<usize> m_nextWorker = 0;
	};
}