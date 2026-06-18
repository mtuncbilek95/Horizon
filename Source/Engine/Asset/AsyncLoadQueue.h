#pragma once

#include <mutex>
#include <vector>

namespace Horizon
{
	struct AsyncQueueCompletion
	{
		u32 slot;
		u32 generation;
		void* asset;
	};

	class AsyncLoadQueue
	{
	public:
		void PushCompleted(const AsyncQueueCompletion& comp)
		{
			std::lock_guard lock(m_mutex);
			m_completed.push_back(comp);
		}

		void DrainCompleted(std::vector<AsyncQueueCompletion>& out)
		{
			out.clear();
			std::lock_guard lock(m_mutex);
			out.swap(m_completed);
		}

		b8 IsEmpty() const
		{
			std::lock_guard lock(m_mutex);
			return m_completed.empty();
		}

	private:
		mutable std::mutex m_mutex;
		std::vector<AsyncQueueCompletion> m_completed;
	};
}