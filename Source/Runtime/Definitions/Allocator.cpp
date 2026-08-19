#include "Allocator.h"

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>

#include <mutex>
#include <unordered_map>
#include <cstddef>

namespace Horizon::Memory
{
	namespace
	{
		struct AllocHeader
		{
			usize size;
			usize align;
		};

		struct MemoryRecord
		{
			usize size;
			usize align;
			SourceLocation location;
		};

		class LeakTracker
		{
		public:
			void OnAllocation(void* p, usize size, usize align, SourceLocation loc)
			{
				std::lock_guard lock(m_mutex);
				m_live.emplace(p, MemoryRecord{ size, align, loc });
				m_liveBytes += size;
			}

			void OnFree(void* p)
			{
				std::lock_guard lock(m_mutex);

				auto it = m_live.find(p);
				if (it == m_live.end())
					return;

				m_liveBytes -= it->second.size;
				m_live.erase(it);
			}

			void Report()
			{
				std::lock_guard lock(m_mutex);
				if (m_live.empty())
				{
					Terminal::Info(StringOps::GetName(this), "No leaks.");
					return;
				}

				Terminal::Error(StringOps::GetName(this), "{} leaks, {} bytes total:", m_live.size(), m_liveBytes);
				for (const auto& [p, r] : m_live)
					Terminal::Error(StringOps::GetName(this), "  {} bytes @ {}:{} ({})",
						r.size, r.location.file_name(), r.location.line(), r.location.function_name());
			}

		private:
			std::unordered_map<void*, MemoryRecord> m_live;
			std::mutex m_mutex;
			usize m_liveBytes = 0;
		};

		LeakTracker& LocalTracker()
		{
			static LeakTracker instance;
			return instance;
		}

		LeakTracker* g_active = nullptr;
		LeakTracker& Tracker() { return g_active ? *g_active : LocalTracker(); }

		usize AlignUp(usize v, usize a) { return (v + (a - 1)) & ~(a - 1); }
	}

	void Memory::Allocator::SetContext(void* tracker) { g_active = static_cast<LeakTracker*>(tracker); }
	void* Memory::Allocator::GetContext() { return &Tracker(); }

	void Memory::Allocator::ReportLeaks() { Tracker().Report(); }

	void* Memory::Allocator::AllocateRaw(usize size, usize align, SourceLocation loc)
	{
		usize eff = align < alignof(std::max_align_t) ? alignof(std::max_align_t) : align;
		usize headerSize = AlignUp(sizeof(AllocHeader), eff);

		void* base = ::operator new(headerSize + size, std::align_val_t{ eff });
		void* user = static_cast<c8*>(base) + headerSize;

		AllocHeader* h = reinterpret_cast<AllocHeader*>(static_cast<c8*>(user) - sizeof(AllocHeader));
		h->size = size;
		h->align = eff;

#if defined(HORIZON_DEBUG)
		Tracker().OnAllocation(user, size, eff, loc);
#endif
		return user;
	}

	void Memory::Allocator::FreeRaw(void* user)
	{
		if (!user)
			return;

		AllocHeader* h = reinterpret_cast<AllocHeader*>(static_cast<c8*>(user) - sizeof(AllocHeader));
		usize eff = h->align;
		usize headerSize = AlignUp(sizeof(AllocHeader), eff);
		void* base = static_cast<c8*>(user) - headerSize;

#if defined(HORIZON_DEBUG)
		Tracker().OnFree(user);
#endif
		::operator delete(base, std::align_val_t{ eff });
	}
}