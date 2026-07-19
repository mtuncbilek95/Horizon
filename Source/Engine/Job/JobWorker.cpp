#include "JobWorker.h"

#include <Engine/Job/JobContext.h>
#include <Runtime/Definitions/Allocator.h>
#include <Runtime/Containers/ScopedLock.h>

#include <utility>

namespace Horizon
{
	void JobWorker::ThreadEntryPoint(void* userData)
	{
		((JobWorker*)userData)->Run();
	}

	JobWorker::JobWorker(JobContext* pContext, usize index) : m_owner(pContext), m_index(index), m_inbox(nullptr),
		m_signal(0), m_working(true)
	{
	}

	JobWorker::~JobWorker()
	{
		JobNode* node = nullptr;
		while (m_deque.PopBottom(node))
			Allocator::Delete(node);

		for (JobNode* nod = m_inbox.Exchange(nullptr); nod;)
		{
			JobNode* next = nod->next;
			Allocator::Delete(nod);
			nod = next;
		}
	}

	void JobWorker::Start()
	{
		m_worker = PAL::Thread(&JobWorker::ThreadEntryPoint, this, "Thread");
	}

	void JobWorker::Run()
	{
		while (m_working.Load())
		{
			DrainInbox();

			Job job;
			if (TryPopJob(job))
			{
				job();
				continue;
			}

			if (auto* pVictim = m_owner->GetRandomVictim(this);
				pVictim && pVictim->TryStealFromThis(job))
			{
				job();
				continue;
			}

			i64 seq = m_signal.Load();

			DrainInbox();
			if (TryPopJob(job))
			{
				job();
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
		JobNode* node = Allocator::Create<JobNode>(CurrLoc(), std::move(job), nullptr);

		JobNode* head = m_inbox.Load();

		while (true)
		{
			node->next = head;
			JobNode* prev = m_inbox.CompareExchange(head, node);

			if (prev == head)
				break;

			head = prev;
		}

		m_signal.FetchAdd(1);
		PAL::Futex::WakeSingle(m_signal.Address());
	}

	b8 JobWorker::TryStealFromThis(Job& out)
	{
		JobNode* node = nullptr;

		if (!m_deque.Steal(node))
			return false;

		out = std::move(node->job);
		Allocator::Delete(node);
		return true;
	}

	void JobWorker::SetThreadAffinity(u64 mask)
	{
		m_worker.SetAffinity(mask);
	}

	void JobWorker::DrainInbox()
	{
		JobNode* head = m_inbox.Exchange(nullptr);
		if (!head)
			return;

		JobNode* ordered = nullptr;
		while (head)
		{
			JobNode* next = head->next;
			head->next = ordered;
			ordered = head;
			head = next;
		}

		for (JobNode* n = ordered; n; )
		{
			JobNode* next = n->next;
			n->next = nullptr;
			m_deque.PushBottom(n);
			n = next;
		}
	}

	b8 JobWorker::TryPopJob(Job& out)
	{
		JobNode* node = nullptr;

		if (!m_deque.PopBottom(node))
			return false;

		out = std::move(node->job);
		Allocator::Delete(node);
		return true;
	}
}