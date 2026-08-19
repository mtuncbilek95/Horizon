// JobSystem.h

#pragma once

#include <Engine/Job/Job.h>
#include <Engine/Job/JobLane.h>
#include <Engine/Job/JobWorker.h>

#include <Runtime/Containers/List.h>
#include <Runtime/PAL/Sync/Atomic.h>

namespace Horizon::Engine
{
	class Engine;

	struct H_EXPORT JobSystemDesc
	{
		u32 criticalWorkers = 0;
		u32 workerWorkers = 0;
		u32 backgroundWorkers = 0;
		u32 maxCriticalWorkers = 4;
		b8 pinMainThread = true;
	};

	class H_EXPORT JobSystem final
	{
		friend class JobWorker;

		struct LaneRuntime
		{
			u32 firstWorker = 0;
			PAL::Atomic<u32> cursor = 0;
			PAL::Atomic<i64> signal = 0;
		};

	public:
		JobSystem(Engine* pEngine, const JobSystemDesc& desc = JobSystemDesc());
		~JobSystem();

		JobSystem(const JobSystem&) = delete;
		JobSystem& operator=(const JobSystem&) = delete;

		void Submit(JobLane lane, Job&& job);
		void Dispatch(JobLane lane, JobCounter& counter, Job&& job);
		void ParallelFor(JobLane lane, usize count, usize grain, ParallelJob body);
		void Wait(JobCounter& counter);

		b8 IsComplete(const JobCounter& counter) const { return counter.remaining.Load() == 0; }
		u32 GetLaneWorkerCount(JobLane lane) const { return m_lanes[(u32)(lane)].workerCount; }

		static JobLane GetCallingLane();

	private:
		b8 BuildTopology();
		void SpawnWorkers();
		void ShutdownWorkers();
		void LogLaneReport() const;

		void WakeLane(JobLane lane);
		b8 TryStealForLane(JobLane lane, Job& out);

		PAL::Atomic<i64>& GetLaneSignal(JobLane lane) { return m_runtime[(u32)(lane)].signal; }

		static void BindCallingThread(JobSystem* pSystem, JobWorker* pWorker, JobLane lane);
		static void UnbindCallingThread();

	private:
		Engine* m_engine;
		JobSystemDesc m_desc;

		List<JobWorker*> m_workers;
		LaneDesc m_lanes[LaneCount];
		LaneRuntime m_runtime[LaneCount];

		u64 m_mainAffinity = 0;
	};
}