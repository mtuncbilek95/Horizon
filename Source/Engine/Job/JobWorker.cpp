#include "JobWorker.h"

#include <Engine/Job/JobSystem.h>

#include <Runtime/Definitions/Allocator.h>
#include <Runtime/PAL/Sync/Futex.h>

#include <format>
#include <string>
#include <utility>

namespace Horizon::Engine
{
	void JobWorker::ThreadEntryPoint(void* userData)
	{
		((JobWorker*)(userData))->Run();
	}

	JobWorker::JobWorker(JobSystem* pSystem, u32 index, JobLane lane) : m_owner(pSystem), m_index(index),
		m_lane(lane), m_affinity(0), m_priority(PAL::ThreadPriority::Normal), m_ecoQoS(false),
		m_inbox(nullptr), m_working(true)
	{
	}

	JobWorker::~JobWorker()
	{
		JobNode* node = nullptr;

		while (m_deque.PopBottom(node))
			Memory::Allocator::Delete(node);

		for (JobNode* nod = m_inbox.Exchange(nullptr); nod;)
		{
			JobNode* next = nod->next;
			Memory::Allocator::Delete(nod);
			nod = next;
		}
	}

	void JobWorker::Configure(u64 affinity, PAL::ThreadPriority priority, b8 ecoQoS)
	{
		m_affinity = affinity;
		m_priority = priority;
		m_ecoQoS = ecoQoS;
	}

	void JobWorker::Start()
	{
		const std::string name = std::format("{}Worker{}", LaneName(m_lane), m_index);
		m_worker = PAL::Thread(&JobWorker::ThreadEntryPoint, this, name);
	}

	void JobWorker::Run()
	{
		PAL::Thread::SetCurrentAffinity(m_affinity);
		PAL::Thread::SetCurrentPriority(m_priority);

		if (m_ecoQoS)
			PAL::Thread::SetCurrentEcoQoS(true);

		JobSystem::BindCallingThread(m_owner, this, m_lane);

		PAL::Atomic<i64>& signal = m_owner->GetLaneSignal(m_lane);

		while (m_working.Load())
		{
			DrainInbox();

			Job job;

			if (TryPopJob(job))
			{
				job();
				continue;
			}

			if (m_owner->TryStealForLane(m_lane, job))
			{
				job();
				continue;
			}

			const i64 seq = signal.Load();

			DrainInbox();

			if (TryPopJob(job))
			{
				job();
				continue;
			}

			if (!m_working.Load())
				break;

			PAL::Futex::Wait(signal.Address(), seq);
		}

		JobSystem::UnbindCallingThread();
	}

	void JobWorker::RequestStop()
	{
		m_working.Store(false);
	}

	void JobWorker::Join()
	{
		if (m_worker.IsJoinable())
			m_worker.Join();
	}

	void JobWorker::AddJob(Job&& job)
	{
		JobNode* node = Memory::Allocator::Create<JobNode>(Memory::CurrLoc(), std::move(job), nullptr);
		JobNode* head = m_inbox.Load();

		while (true)
		{
			node->next = head;
			JobNode* prev = m_inbox.CompareExchange(head, node);

			if (prev == head)
				break;

			head = prev;
		}
	}

	void JobWorker::PushLocal(Job&& job)
	{
		JobNode* node = Memory::Allocator::Create<JobNode>(Memory::CurrLoc(), std::move(job), nullptr);
		m_deque.PushBottom(node);
	}

	b8 JobWorker::TryStealFromThis(Job& out)
	{
		JobNode* node = nullptr;

		if (!m_deque.Steal(node))
			return false;

		out = std::move(node->job);
		Memory::Allocator::Delete(node);
		return true;
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

		for (JobNode* node = ordered; node;)
		{
			JobNode* next = node->next;
			node->next = nullptr;
			m_deque.PushBottom(node);
			node = next;
		}
	}

	b8 JobWorker::TryPopJob(Job& out)
	{
		JobNode* node = nullptr;

		if (!m_deque.PopBottom(node))
			return false;

		out = std::move(node->job);
		Memory::Allocator::Delete(node);
		return true;
	}
}