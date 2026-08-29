#include "Allocator.h"

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>

#include <cstring>
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
			u32 align;
			u32 line;
			const c8* pFile;
		};

		class LeakTracker
		{
		public:
			void OnAllocation(void* pAddress, usize size, usize align, SourceLocation loc)
			{
				Shard& shard = ShardFor(pAddress);
				std::lock_guard lock(shard.mutex);

				shard.live.emplace(pAddress, MemoryRecord{ size, u32(align), loc.line(), loc.file_name() });
				shard.liveBytes += size;
			}

			void OnFree(void* pAddress)
			{
				Shard& shard = ShardFor(pAddress);
				std::lock_guard lock(shard.mutex);

				auto it = shard.live.find(pAddress);

				if (it == shard.live.end())
					return;

				shard.liveBytes -= it->second.size;
				shard.live.erase(it);
			}

			void Report()
			{
				usize totalCount = 0;
				usize totalBytes = 0;

				for (Shard& shard : m_shards)
				{
					std::lock_guard lock(shard.mutex);
					totalCount += shard.live.size();
					totalBytes += shard.liveBytes;
				}

				if (totalCount == 0)
				{
					Terminal::Info("LeakTracker", "No leaks.");
					return;
				}

				Terminal::Error("LeakTracker", "{} leaks, {} bytes total:", totalCount, totalBytes);

				for (Shard& shard : m_shards)
				{
					std::lock_guard lock(shard.mutex);

					for (const auto& [pAddress, record] : shard.live)
						Terminal::Error("LeakTracker", "  {} bytes @ {}:{}", record.size, record.pFile, record.line);
				}
			}

		private:
			static constexpr usize ShardCount = 16;

			struct Shard
			{
				std::unordered_map<void*, MemoryRecord> live;
				std::mutex mutex;
				usize liveBytes = 0;
			};

			Shard& ShardFor(void* pAddress)
			{
				const uptr value = reinterpret_cast<uptr>(pAddress);
				return m_shards[(value >> 4) & (ShardCount - 1)];
			}

			Shard m_shards[ShardCount];
		};

		LeakTracker& Tracker()
		{
			static LeakTracker instance;
			return instance;
		}

		usize AlignUp(usize value, usize align) { return (value + (align - 1)) & ~(align - 1); }
	}

	b8 Allocator::IsTrackingEnabled()
	{
#if defined(HORIZON_MEMORY_TRACKING)
		return true;
#else
		return false;
#endif
	}

	void Allocator::ReportLeaks()
	{
		if (!IsTrackingEnabled())
		{
			Terminal::Warn("LeakTracker", "Memory tracking is disabled, leak state is unknown.");
			return;
		}

		Tracker().Report();
	}

	void* Allocator::AllocateRaw(usize size, usize align, SourceLocation loc)
	{
		const usize eff = align < alignof(std::max_align_t) ? alignof(std::max_align_t) : align;
		const usize headerSize = AlignUp(sizeof(AllocHeader), eff);

		void* base = ::operator new(headerSize + size, std::align_val_t{ eff }, std::nothrow);

		if (!base)
		{
			Terminal::Fatal("Allocator", "Out of memory requesting {} bytes", size);
			return nullptr;
		}

		void* user = static_cast<c8*>(base) + headerSize;

		AllocHeader* pHeader = reinterpret_cast<AllocHeader*>(static_cast<c8*>(user) - sizeof(AllocHeader));
		pHeader->size = size;
		pHeader->align = eff;

#if defined(HORIZON_MEMORY_TRACKING)
		Tracker().OnAllocation(user, size, eff, loc);
#endif
		return user;
	}

	void* Allocator::ReallocateRaw(void* pAddress, usize newSize, usize align, SourceLocation loc)
	{
		if (!pAddress)
			return AllocateRaw(newSize, align, loc);

		if (newSize == 0)
		{
			FreeRaw(pAddress);
			return nullptr;
		}

		const AllocHeader* pHeader = reinterpret_cast<const AllocHeader*>(static_cast<c8*>(pAddress) - sizeof(AllocHeader));
		const usize oldSize = pHeader->size;

		void* pNew = AllocateRaw(newSize, align, loc);

		if (!pNew)
			return nullptr;

		std::memcpy(pNew, pAddress, oldSize < newSize ? oldSize : newSize);
		FreeRaw(pAddress);

		return pNew;
	}

	void Allocator::FreeRaw(void* pAddress)
	{
		if (!pAddress)
			return;

		AllocHeader* pHeader = reinterpret_cast<AllocHeader*>(static_cast<c8*>(pAddress) - sizeof(AllocHeader));
		const usize eff = pHeader->align;
		const usize headerSize = AlignUp(sizeof(AllocHeader), eff);
		void* base = static_cast<c8*>(pAddress) - headerSize;

#if defined(HORIZON_MEMORY_TRACKING)
		Tracker().OnFree(pAddress);
#endif
		::operator delete(base, std::align_val_t{ eff });
	}
}