#pragma once

#include <Engine/Engine/IModule.h>
#include <Engine/Job/Job.h>
#include <Engine/Job/ThreadPool.h>

namespace Horizon
{
	class JobModule : public IModule
	{
	public:
		void OnAttach(Engine& engine) final;
		void OnDetach() final;

		void Submit(JobFunc job);
		void Submit(JobCounter& counter, JobFunc job);
		void Dispatch(JobCounter& counter, u32 count, u32 groupSize, JobRangeFunc fn);
		void Wait(JobCounter& counter);

		u32 WorkerCount() const { return m_workerCount; }
		u32 WorkerIndex() const { return ThreadPool::LocalWorkerIndex(); }

	private:
		ThreadPool m_pool;
		u32 m_workerCount = 0;
	};
}