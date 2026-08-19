#include "JobSystem.h"

#include <Runtime/Definitions/Allocator.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/Hardware/Processor.h>
#include <Runtime/PAL/Sync/Futex.h>
#include <Runtime/PAL/Sync/Thread.h>

#include <algorithm>
#include <random>
#include <utility>
#include <string_view>

namespace Horizon::Engine
{
	namespace
	{
		thread_local JobSystem* t_system = nullptr;
		thread_local JobWorker* t_worker = nullptr;
		thread_local JobLane t_lane = JobLane::Count;

		struct PhysicalCore
		{
			u32 index = 0;
			b8 isPerformance = false;
			u64 mask = 0;
			u32 primaryLogical = 0;
			u32 logicalCount = 0;
		};

		List<PhysicalCore> GroupPhysicalCores(const List<PAL::CoreInfo>& cores)
		{
			List<PhysicalCore> result;

			for (const PAL::CoreInfo& core : cores)
			{
				PhysicalCore* pFound = nullptr;

				for (PhysicalCore& phys : result)
				{
					if (phys.index == core.physicalIndex)
					{
						pFound = &phys;
						break;
					}
				}

				if (!pFound)
				{
					PhysicalCore& fresh = result.EmplaceBack();
					fresh.index = core.physicalIndex;
					fresh.isPerformance = core.isPerformance;
					fresh.primaryLogical = core.logicalIndex;
					pFound = &fresh;
				}

				pFound->mask |= (1ull << core.logicalIndex);
				pFound->logicalCount++;
			}

			return result;
		}

		u64 NthSetBit(u64 mask, u32 nth)
		{
			for (u32 bit = 0; bit < 64; ++bit)
			{
				if (!(mask & (1ull << bit)))
					continue;

				if (nth == 0)
					return 1ull << bit;

				--nth;
			}

			Terminal::Warn("JobSystem", "Mask 0x{:X} has no bit at position {}, falling back to the full mask", mask, nth);
			return mask;
		}

		std::string FormatCoreList(u64 mask)
		{
			std::string text;

			for (u32 bit = 0; bit < 64; ++bit)
			{
				if (!(mask & (1ull << bit)))
					continue;

				if (!text.empty())
					text += ", ";

				text += std::to_string(bit);
			}

			if (text.empty())
				return "none";

			return text;
		}

		std::string FormatLaneList(u32 laneMask)
		{
			std::string text;

			for (u32 lane = 0; lane < LaneCount; ++lane)
			{
				if (!(laneMask & (1u << lane)))
					continue;

				if (!text.empty())
					text += ", ";

				text += LaneName((JobLane)(lane));
			}

			if (text.empty())
				return "none";

			return text;
		}

		std::string_view PriorityName(PAL::ThreadPriority priority)
		{
			switch (priority)
			{
			case PAL::ThreadPriority::Low:
				return "Low";
			case PAL::ThreadPriority::Normal:
				return "Normal";
			case PAL::ThreadPriority::High:
				return "High";
			case PAL::ThreadPriority::Highest:
				return "Highest";
			case PAL::ThreadPriority::Realtime:
				return "Realtime";
			default:
				return "Unknown";
			}
		}
	}

	JobSystem::JobSystem(Engine* pEngine, const JobSystemDesc& desc) : m_engine(pEngine), m_desc(desc)
	{
		if (!BuildTopology())
		{
			Terminal::Fatal("JobSystem", "Core topology could not be resolved, no worker has been created");
			return;
		}

		SpawnWorkers();

		if (m_desc.pinMainThread && m_mainAffinity != 0)
		{
			PAL::Thread::SetCurrentAffinity(m_mainAffinity);
			PAL::Thread::SetCurrentPriority(PAL::ThreadPriority::High);
		}

		BindCallingThread(this, nullptr, JobLane::Critical);
		LogLaneReport();
	}

	JobSystem::~JobSystem()
	{
		ShutdownWorkers();
		UnbindCallingThread();
	}

	b8 JobSystem::BuildTopology()
	{
		const List<PAL::CoreInfo> cores = PAL::Processor::EnumerateCores();

		if (cores.IsEmpty())
		{
			Terminal::Error("JobSystem", "Processor enumeration returned no core");
			return false;
		}

		const List<PhysicalCore> physical = GroupPhysicalCores(cores);
		List<PhysicalCore> perf;
		List<PhysicalCore> eff;

		for (const PhysicalCore& phys : physical)
		{
			if (phys.isPerformance)
				perf.PushBack(phys);
			else
				eff.PushBack(phys);
		}

		if (perf.IsEmpty())
		{
			Terminal::Warn("JobSystem", "No performance class core reported, the whole core set is treated as performance");
			perf = physical;
			eff.Clear();
		}

		usize cursor = 0;

		if (m_desc.pinMainThread && perf.GetCount() > 1)
		{
			m_mainAffinity = 1ull << perf[0].primaryLogical;
			cursor = 1;
		}

		const usize availablePerf = perf.GetCount() - cursor;

		u32 criticalCount = m_desc.criticalWorkers;

		if (criticalCount == 0)
			criticalCount = (u32)(std::clamp<usize>(availablePerf / 2, 1, m_desc.maxCriticalWorkers));

		if (availablePerf >= 2)
			criticalCount = (u32)(std::min<usize>(criticalCount, availablePerf - 1));
		else
			criticalCount = 1;

		u64 criticalMask = 0;

		for (u32 i = 0; i < criticalCount && cursor < perf.GetCount(); ++i, ++cursor)
			criticalMask |= (1ull << perf[cursor].primaryLogical);

		u64 workerMask = 0;
		u32 workerCount = 0;

		for (usize i = cursor; i < perf.GetCount(); ++i)
		{
			workerMask |= perf[i].mask;
			workerCount += perf[i].logicalCount;
		}

		if (workerCount == 0)
		{
			Terminal::Warn("JobSystem", "Worker lane has no dedicated core left, it shares the critical cores");
			workerMask = criticalMask;
			workerCount = 1;
		}

		u64 backgroundMask = 0;
		u32 backgroundCount = 0;

		for (const PhysicalCore& phys : eff)
		{
			backgroundMask |= phys.mask;
			backgroundCount += phys.logicalCount;
		}

		if (backgroundCount == 0)
		{
			Terminal::Warn("JobSystem", "No efficiency core reported, background lane runs on the worker cores with low priority");
			backgroundMask = workerMask;
			backgroundCount = std::max<u32>(1, workerCount / 4);
		}

		if (m_desc.workerWorkers != 0)
			workerCount = m_desc.workerWorkers;

		if (m_desc.backgroundWorkers != 0)
			backgroundCount = m_desc.backgroundWorkers;

		m_lanes[(u32)(JobLane::Critical)] = LaneDesc
		{
			criticalCount,
			LaneBit(JobLane::Critical) | LaneBit(JobLane::Worker),
			LaneBit(JobLane::Critical),
			criticalMask,
			true,
			false,
			PAL::ThreadPriority::High
		};

		m_lanes[(u32)(JobLane::Worker)] = LaneDesc
		{
			workerCount,
			LaneBit(JobLane::Worker) | LaneBit(JobLane::Background),
			LaneBit(JobLane::Worker) | LaneBit(JobLane::Critical),
			workerMask,
			false,
			false,
			PAL::ThreadPriority::Normal
		};

		m_lanes[(u32)(JobLane::Background)] = LaneDesc
		{
			backgroundCount,
			LaneBit(JobLane::Background),
			LaneBit(JobLane::Background) | LaneBit(JobLane::Worker),
			backgroundMask,
			false,
			true,
			PAL::ThreadPriority::Low
		};

		u32 offset = 0;

		for (u32 lane = 0; lane < LaneCount; ++lane)
		{
			m_runtime[lane].firstWorker = offset;
			offset += m_lanes[lane].workerCount;
		}

		return true;
	}

	void JobSystem::SpawnWorkers()
	{
		for (u32 lane = 0; lane < LaneCount; ++lane)
		{
			const LaneDesc& desc = m_lanes[lane];

			for (u32 i = 0; i < desc.workerCount; ++i)
			{
				const u32 index = (u32)(m_workers.GetCount());
				const u64 affinity = desc.exclusiveCore ? NthSetBit(desc.laneAffinity, i) : desc.laneAffinity;

				JobWorker* pWorker = Memory::Allocator::Create<JobWorker>(Memory::CurrLoc(), this, index, (JobLane)(lane));
				pWorker->Configure(affinity, desc.priority, desc.ecoQoS);

				m_workers.PushBack(pWorker);
			}
		}

		for (JobWorker* pWorker : m_workers)
			pWorker->Start();
	}

	void JobSystem::ShutdownWorkers()
	{
		for (JobWorker* pWorker : m_workers)
			pWorker->RequestStop();

		for (u32 lane = 0; lane < LaneCount; ++lane)
		{
			m_runtime[lane].signal.FetchAdd(1);
			PAL::Futex::WakeAll(m_runtime[lane].signal.Address());
		}

		for (JobWorker* pWorker : m_workers)
			pWorker->Join();

		for (JobWorker* pWorker : m_workers)
			Memory::Allocator::Delete(pWorker);

		m_workers.Clear();
	}

	void JobSystem::Submit(JobLane lane, Job&& job)
	{
		const u32 laneIndex = (u32)(lane);

		if (laneIndex >= LaneCount)
		{
			Terminal::Error("JobSystem", "Submit received an invalid lane, the job runs inline");
			job();
			return;
		}

		const LaneDesc& desc = m_lanes[laneIndex];

		if (desc.workerCount == 0)
		{
			Terminal::Error("JobSystem", "{} lane owns no worker, the job runs inline", LaneName(lane));
			job();
			return;
		}

		if (t_system == this && t_worker && t_worker->GetLane() == lane)
		{
			t_worker->PushLocal(std::move(job));
			WakeLane(lane);
			return;
		}

		LaneRuntime& runtime = m_runtime[laneIndex];
		const u32 slot = runtime.firstWorker + (runtime.cursor.FetchAdd(1) % desc.workerCount);

		m_workers[slot]->AddJob(std::move(job));
		WakeLane(lane);
	}

	void JobSystem::Dispatch(JobLane lane, JobCounter& counter, Job&& job)
	{
		counter.lane.Store((u32)(lane));
		counter.remaining.FetchAdd(1);

		Submit(lane, [&counter, job = std::move(job)]()
			{
				job();
				counter.remaining.FetchSubtract(1);
			});
	}

	void JobSystem::ParallelFor(JobLane lane, usize count, usize grain, ParallelJob body)
	{
		if (count == 0)
			return;

		if (grain == 0)
			grain = 1;

		JobCounter counter;

		for (usize begin = 0; begin < count; begin += grain)
		{
			const usize end = std::min(begin + grain, count);

			Dispatch(lane, counter, [body, begin, end]()
				{
					body(begin, end);
				});
		}

		Wait(counter);
	}

	void JobSystem::Wait(JobCounter& counter)
	{
		const JobLane callerLane = t_lane;
		const JobLane counterLane = (JobLane)(counter.lane.Load());

		b8 canAssist = (t_system == this && callerLane != JobLane::Count);

		if (!canAssist && counter.remaining.Load() > 0)
			Terminal::Warn("JobSystem", "Wait was called from a thread with no lane binding, it can only block");

		if (canAssist && counterLane != JobLane::Count &&
			!(m_lanes[(u32)(callerLane)].stealMask & LaneBit(counterLane)))
		{
			Terminal::Error("JobSystem", "{} lane is not allowed to execute {} lane work, it blocks instead of assisting",
				LaneName(callerLane), LaneName(counterLane));
			canAssist = false;
		}

		while (counter.remaining.Load() > 0)
		{
			Job job;

			if (canAssist && TryStealForLane(callerLane, job))
			{
				job();
				continue;
			}

			PAL::Thread::YieldCurrent();
		}
	}

	void JobSystem::WakeLane(JobLane lane)
	{
		const u32 wakeMask = m_lanes[(u32)(lane)].wakeMask;

		for (u32 target = 0; target < LaneCount; ++target)
		{
			if (!(wakeMask & (1u << target)))
				continue;

			m_runtime[target].signal.FetchAdd(1);
			PAL::Futex::WakeSingle(m_runtime[target].signal.Address());
		}
	}

	b8 JobSystem::TryStealForLane(JobLane lane, Job& out)
	{
		const u32 stealMask = m_lanes[(u32)(lane)].stealMask;

		thread_local std::mt19937 random{ std::random_device{}() };

		for (u32 target = 0; target < LaneCount; ++target)
		{
			if (!(stealMask & (1u << target)))
				continue;

			const u32 count = m_lanes[target].workerCount;

			if (count == 0)
				continue;

			const u32 first = m_runtime[target].firstWorker;
			const u32 start = (u32)(random() % count);

			for (u32 i = 0; i < count; ++i)
			{
				JobWorker* pVictim = m_workers[first + ((start + i) % count)];

				if (pVictim == t_worker)
					continue;

				if (pVictim->TryStealFromThis(out))
					return true;
			}
		}

		return false;
	}

	void JobSystem::LogLaneReport() const
	{
		Terminal::Info("JobSystem", "Main thread pinned to logical core {}", FormatCoreList(m_mainAffinity));

		for (u32 lane = 0; lane < LaneCount; ++lane)
		{
			const LaneDesc& desc = m_lanes[lane];

			Terminal::Info("JobSystem", "{} lane | {} worker | {} priority | pool {} | {} | steals from {}",
				LaneName((JobLane)(lane)), desc.workerCount, PriorityName(desc.priority),
				FormatCoreList(desc.laneAffinity),
				desc.exclusiveCore ? "one core per worker" : "shared pool",
				FormatLaneList(desc.stealMask));

			const LaneRuntime& runtime = m_runtime[lane];

			for (u32 i = 0; i < desc.workerCount; ++i)
			{
				const JobWorker* pWorker = m_workers[runtime.firstWorker + i];

				Terminal::Debug("JobSystem", "  {}Worker{} on logical core {}",
					LaneName((JobLane)(lane)), pWorker->GetWorkerIndex(), FormatCoreList(pWorker->GetAffinity()));
			}
		}
	}

	void JobSystem::BindCallingThread(JobSystem* pSystem, JobWorker* pWorker, JobLane lane)
	{
		t_system = pSystem;
		t_worker = pWorker;
		t_lane = lane;
	}

	void JobSystem::UnbindCallingThread()
	{
		t_system = nullptr;
		t_worker = nullptr;
		t_lane = JobLane::Count;
	}

	JobLane JobSystem::GetCallingLane()
	{
		return t_lane;
	}
}