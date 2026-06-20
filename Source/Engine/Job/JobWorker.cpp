#include "JobWorker.h"

#include <Engine/Job./JobModule.h>

namespace Horizon
{
	JobWorker::JobWorker(JobModule* pModule, usize index) : m_owner(pModule),
		m_index(index), m_worker(&JobWorker::Run, this), m_working(true)
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

			std::unique_lock lock(m_mutex);
			m_condition.wait_for(lock, std::chrono::milliseconds(1),
				[&] { return !m_jobs.empty() || !m_working; });
		}
	}

	void JobWorker::Stop()
	{
		m_working = false;
		m_condition.notify_one();

		if (m_worker.joinable())
			m_worker.join();
	}

	void JobWorker::AddJob(Job&& job)
	{
		{
			std::lock_guard lock(m_mutex);
			m_jobs.push_back(std::move(job));
		}

		m_condition.notify_one();
	}

	b8 JobWorker::TryStealFromThis(Job& out)
	{
		std::lock_guard lock(m_mutex);

		if (m_jobs.empty())
			return false;

		out = std::move(m_jobs.front());
		m_jobs.pop_front();

		return true;
	}

	b8 JobWorker::TryPopJob(Job& out)
	{
		std::lock_guard lock(m_mutex);

		if (m_jobs.empty())
			return false;

		out = std::move(m_jobs.back());
		m_jobs.pop_back();

		return true;
	}
}