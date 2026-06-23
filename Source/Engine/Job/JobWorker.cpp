#include "JobWorker.h"

#include <Engine/Job./JobModule.h>

#include <Runtime/Containers/ScopedLock.h>

namespace Horizon
{
	void JobWorker::ThreadEntryPoint(void* userData)
	{
		((JobWorker*)userData)->Run();
	}

	JobWorker::JobWorker(JobModule* pModule, usize index) : m_owner(pModule),
		m_index(index), m_worker(&JobWorker::ThreadEntryPoint, this, "Thread"), m_working(true)
	{
	}

	void JobWorker::Run()
	{
		while (m_working)
		{
			Job currJob;
			if (TryPopJob(currJob))
			{
				currJob();
				continue;
			}

			if (auto* pVictim = m_owner->GetRandomVictim(this);
				pVictim && pVictim->TryStealFromThis(currJob))
			{
				currJob();
				continue;
			}

			m_signal.TryAcquire(1);
		}
	}

	void JobWorker::Stop()
	{
		m_working = false;
		m_signal.Release();

		if (m_worker.IsJoinable())
			m_worker.Join();
	}

	void JobWorker::AddJob(Job&& job)
	{
		{
			ScopedLock lock(m_mutex);
			m_jobs.push_back(std::move(job));
		}

		m_signal.Release();
	}

	b8 JobWorker::TryStealFromThis(Job& out)
	{
		ScopedLock lock(m_mutex);

		if (m_jobs.empty())
			return false;

		out = std::move(m_jobs.front());
		m_jobs.pop_front();

		return true;
	}

	void JobWorker::SetThreadAffinity(u64 mask)
	{
		m_worker.SetAffinity(mask);
	}

	b8 JobWorker::TryPopJob(Job& out)
	{
		ScopedLock lock(m_mutex);

		if (m_jobs.empty())
			return false;

		out = std::move(m_jobs.back());
		m_jobs.pop_back();

		return true;
	}
}