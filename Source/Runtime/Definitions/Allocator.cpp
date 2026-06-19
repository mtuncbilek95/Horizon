#include "Allocator.h"

#include <Runtime/Log/Terminal.h>

#include <mutex>
#include <unordered_map>

namespace Horizon
{
	namespace
	{
		struct MemoryRecord
		{
			usize size;
			usize align;
			SourceLocation location;
		};

		class LeakTracker
		{
		public:
			void OnAllocation(void* pAdress, usize size, usize align, SourceLocation loc)
			{
				std::lock_guard lock(m_mutex);
				m_live.emplace(pAdress, MemoryRecord{ size, align, loc });
				m_liveBytes += size;
			}

			MemoryRecord OnFree(void* pAdress)
			{
				std::lock_guard lock(m_mutex);

				auto it = m_live.find(pAdress);

				MemoryRecord rec = it->second;
				m_liveBytes -= rec.size;
				m_live.erase(it);

				return rec;
			}

			void Report()
			{
				std::lock_guard lock(m_mutex);
				if (m_live.empty())
				{
					Terminal::Info("Allocator", "No leaks. {} live allocations.", m_live.size());
					return;
				}

				Terminal::Error("Allocator", "{} leaks, {} bytes total:", m_live.size(), m_liveBytes);
				for (const auto& [p, r] : m_live)
					Terminal::Error("Allocator", "  {} bytes @ {}:{} ({})",
						r.size, r.location.file_name(), r.location.line(), r.location.function_name());
			}

		private:
			std::unordered_map<void*, MemoryRecord> m_live;
			std::mutex m_mutex;

			usize m_liveBytes = 0;
		};

		LeakTracker& Tracker()
		{
			static LeakTracker instance;
			return instance;
		}
	}

	void Allocator::ReportLeaks()
	{
		Tracker().Report();
	}

	void* Allocator::AllocateRaw(usize size, usize align, SourceLocation loc)
	{
		void* pAdr = ::operator new(size, std::align_val_t{ align });

#if defined(HORIZON_DEBUG)
		Tracker().OnAllocation(pAdr, size, align, loc);
#endif
		return pAdr;
	}

	void Allocator::FreeRaw(void* pAddress)
	{
#if defined(HORIZON_DEBUG)
		MemoryRecord rec = Tracker().OnFree(pAddress);
#endif
		::operator delete(pAddress, std::align_val_t{ rec.align });
	}
}