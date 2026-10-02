#include "JobWorker.h"

#include <Engine/Job/JobSystem.h>
#include <Runtime/Definitions/Allocator.h>

#include <utility>

namespace Horizon::Engine
{
	void JobWorker::ThreadEntryPoint(void* pUserData)
	{
		((JobWorker*)pUserData)->Run();
	}

	JobWorker::JobWorker(JobSystem* pContext, JobLane lane, usize index) : m_owner(pContext), m_lane(lane), m_index(index),
		m_inbox(nullptr), m_signal(0), m_working(true)
	{
	}

	JobWorker::~JobWorker()
	{
		JobNode* pNode = nullptr;
		while (m_deque.PopBottom(pNode))
			Memory::Allocator::Delete(pNode);

		for (JobNode* pCurrent = m_inbox.Exchange(nullptr); pCurrent;)
		{
			JobNode* pNext = pCurrent->pNext;
			Memory::Allocator::Delete(pCurrent);
			pCurrent = pNext;
		}
	}

	void JobWorker::Start()
	{
		m_worker = PAL::Thread(&JobWorker::ThreadEntryPoint, this,
			m_lane == JobLane::Critical ? "CriticalWorker" : "BackgroundWorker");
	}

	void JobWorker::Run()
	{
		if (m_lane == JobLane::Background)
			PAL::Thread::SetCurrentPriority(PAL::ThreadPriority::Normal);
		else
			PAL::Thread::SetCurrentPriority(PAL::ThreadPriority::Highest);

		while (m_working.Load())
		{
			DrainInbox();

			Job job;
			if (TryPopJob(job))
			{
				job.Execute();
				continue;
			}

			if (auto* pVictim = m_owner->GetRandomVictim(this);
				pVictim && pVictim->TryStealFromThis(job))
			{
				job.Execute();
				continue;
			}

			i64 seq = m_signal.Load();

			DrainInbox();
			if (TryPopJob(job))
			{
				job.Execute();
				continue;
			}

			if (!m_working.Load())
				break;

			PAL::Futex::Wait(m_signal.Address(), seq);
		}
	}

	void JobWorker::Stop()
	{
		m_working.Store(false);

		m_signal.FetchAdd(1);
		PAL::Futex::WakeAll(m_signal.Address());

		if (m_worker.IsJoinable())
			m_worker.Join();
	}

	void JobWorker::AddJob(Job&& job)
	{
		JobNode* pNode = Memory::Allocator::Create<JobNode>(Memory::CurrLoc(), std::move(job));

		JobNode* pHead = m_inbox.Load();

		while (true)
		{
			pNode->pNext = pHead;
			JobNode* pPrev = m_inbox.CompareExchange(pHead, pNode);

			if (pPrev == pHead)
				break;

			pHead = pPrev;
		}

		m_signal.FetchAdd(1);
		PAL::Futex::WakeSingle(m_signal.Address());
	}

	b8 JobWorker::TryStealFromThis(Job& out)
	{
		JobNode* pNode = nullptr;

		if (!m_deque.Steal(pNode))
			return false;

		out = std::move(pNode->job);
		Memory::Allocator::Delete(pNode);
		return true;
	}

	void JobWorker::SetThreadAffinity(u64 mask)
	{
		m_worker.SetAffinity(mask);
	}

	void JobWorker::DrainInbox()
	{
		JobNode* pHead = m_inbox.Exchange(nullptr);
		if (!pHead)
			return;

		JobNode* pOrdered = nullptr;
		while (pHead)
		{
			JobNode* pNext = pHead->pNext;
			pHead->pNext = pOrdered;
			pOrdered = pHead;
			pHead = pNext;
		}

		for (JobNode* pCurrent = pOrdered; pCurrent; )
		{
			JobNode* pNext = pCurrent->pNext;
			pCurrent->pNext = nullptr;
			m_deque.PushBottom(pCurrent);
			pCurrent = pNext;
		}
	}

	b8 JobWorker::TryPopJob(Job& out)
	{
		JobNode* pNode = nullptr;

		if (!m_deque.PopBottom(pNode))
			return false;

		out = std::move(pNode->job);
		Memory::Allocator::Delete(pNode);
		return true;
	}
}