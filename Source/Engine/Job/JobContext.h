#pragma once

#include <Engine/Core/Context.h>
#include <Engine/Job/Job.h>
#include <Engine/Job/JobWorker.h>

#include <Runtime/PAL/Sync/Atomic.h>
#include <Runtime/Containers/List.h>

namespace Horizon
{
	class H_EXPORT JobContext : public Context
	{
		friend class JobWorker;
	public:
		EngineReport OnAttach(Engine* pEngine) final;
		void OnDetach() final;

		void SubmitJob(Job&& job);
		void Dispatch(JobCounter& counter, Job&& job);
		void Wait(JobCounter& counter);

		void GetInitializeOrder(OrderRules& rules) const final;

	private:
		JobWorker* GetRandomVictim(JobWorker* avoidWorker);
		b8 TryRunOneJob();

	private:
		List<JobWorker*> m_workers;
		PAL::Atomic<usize> m_nextWorker = 0;
	};
}