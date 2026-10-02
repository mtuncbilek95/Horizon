#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/Definitions/Allocator.h>
#include <Runtime/PAL/Sync/Atomic.h>
#include <Runtime/Containers/List.h>

namespace Horizon
{
	template<typename T>
	class WorkStealingDeque
	{
		struct Buffer
		{
			Buffer(i64 capacity) : mask(capacity - 1),
				pSlots((T*)Memory::Allocator::AllocateRaw(sizeof(T) * (usize)capacity, alignof(T), Memory::CurrLoc()))
			{
				for (i64 i = 0; i < capacity; ++i)
					::new (pSlots + i) T();
			}

			~Buffer()
			{
				for (i64 i = 0; i <= mask; ++i)
					pSlots[i].~T();

				Memory::Allocator::FreeRaw(pSlots);
			}

			i64 Capacity() const { return mask + 1; }
			T Get(i64 index) const { return pSlots[index & mask]; }
			void Put(i64 index, T value) { pSlots[index & mask] = value; }

			i64 mask;
			T* pSlots;
		};
	public:
		WorkStealingDeque(i64 capacity = 256) : m_top(0), m_bottom(0),
			m_buffer(Memory::Allocator::Create<Buffer>(Memory::CurrLoc(), capacity))
		{
		}

		~WorkStealingDeque()
		{
			for (Buffer* pOld : m_retired)
				Memory::Allocator::Delete(pOld);
			Memory::Allocator::Delete(m_buffer.Load());
		}

		WorkStealingDeque(const WorkStealingDeque&) = delete;
		WorkStealingDeque& operator=(const WorkStealingDeque&) = delete;

		void PushBottom(T value)
		{
			i64 bottom = m_bottom.Load();
			i64 top = m_top.Load();
			Buffer* pBuffer = m_buffer.Load();

			if (bottom - top >= pBuffer->Capacity())
				pBuffer = Grow(pBuffer, bottom, top);

			pBuffer->Put(bottom, value);
			m_bottom.Store(bottom + 1);
		}

		b8 PopBottom(T& out)
		{
			i64 bottom = m_bottom.Load() - 1;
			Buffer* pBuffer = m_buffer.Load();
			m_bottom.Store(bottom);
			i64 top = m_top.Load();

			if (top > bottom)
			{
				m_bottom.Store(bottom + 1);
				return false;
			}

			out = pBuffer->Get(bottom);
			if (top != bottom)
				return true;

			b8 won = (m_top.CompareExchange(top, top + 1) == top);
			m_bottom.Store(bottom + 1);
			return won;
		}

		b8 Steal(T& out)
		{
			i64 top = m_top.Load();
			i64 bottom = m_bottom.Load();

			if (top >= bottom)
				return false;

			Buffer* pBuffer = m_buffer.Load();
			out = pBuffer->Get(top);

			if (m_top.CompareExchange(top, top + 1) != top)
				return false;

			return true;
		}

		b8 IsEmpty() const { return m_bottom.Load() <= m_top.Load(); }

	private:
		Buffer* Grow(Buffer* pOld, i64 bottom, i64 top)
		{
			Buffer* pFresh = Memory::Allocator::Create<Buffer>(Memory::CurrLoc(), pOld->Capacity() * 2);

			for (i64 i = top; i < bottom; ++i)
				pFresh->Put(i, pOld->Get(i));

			m_buffer.Store(pFresh);
			m_retired.PushBack(pOld);

			return pFresh;
		}
	private:
		PAL::Atomic<i64> m_top;
		PAL::Atomic<i64> m_bottom;
		PAL::Atomic<Buffer*> m_buffer;
		List<Buffer*> m_retired;
	};
}