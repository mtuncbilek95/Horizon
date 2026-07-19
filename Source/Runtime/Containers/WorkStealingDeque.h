#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/PAL/Sync/Atomic.h>

#include <vector>

namespace Horizon
{
	template<typename T>
	class WorkStealingDeque
	{
		struct Buffer
		{
			Buffer(i64 capacity) : mask(capacity - 1), slots(new T[capacity]()) {}
			~Buffer() { delete[] slots; }

			i64 Capacity() const { return mask + 1; }
			T Get(i64 index) const { return slots[index & mask]; }
			void Put(i64 index, T value) { slots[index & mask] = value; }

			i64 mask;
			T* slots;
		};
	public:
		WorkStealingDeque(i64 capacity = 256) : m_top(0), m_bottom(0), 
			m_buffer(new Buffer(capacity))
		{
		}

		~WorkStealingDeque()
		{
			for (Buffer* old : m_retired)
				delete old;
			delete m_buffer.Load();
		}

		WorkStealingDeque(const WorkStealingDeque&) = delete;
		WorkStealingDeque& operator=(const WorkStealingDeque&) = delete;

		void PushBottom(T value)
		{
			i64 bottom = m_bottom.Load();
			i64 top = m_top.Load();
			Buffer* buf = m_buffer.Load();

			if (bottom - top >= buf->Capacity())
				buf = Grow(buf, bottom, top);

			buf->Put(bottom, value);
			m_bottom.Store(bottom + 1);
		}

		b8 PopBottom(T& out)
		{
			i64 bottom = m_bottom.Load() - 1;
			Buffer* buf = m_buffer.Load();
			m_bottom.Store(bottom);
			i64 top = m_top.Load();

			if (top > bottom)
			{
				m_bottom.Store(bottom + 1);
				return false;
			}

			out = buf->Get(bottom);
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

			Buffer* buf = m_buffer.Load();
			out = buf->Get(top);

			if (m_top.CompareExchange(top, top + 1) != top)
				return false;

			return true;
		}

		b8 IsEmpty() const { return m_bottom.Load() <= m_top.Load(); }

	private:
		Buffer* Grow(Buffer* old, i64 bottom, i64 top)
		{
			Buffer* fresh = new Buffer(old->Capacity() * 2);

			for (i64 i = top; i < bottom; ++i)
				fresh->Put(i, old->Get(i));

			m_buffer.Store(fresh);
			m_retired.push_back(old);

			return fresh;
		}
	private:
		PAL::Atomic<i64> m_top;
		PAL::Atomic<i64> m_bottom;
		PAL::Atomic<Buffer*> m_buffer;
		std::vector<Buffer*> m_retired;
	};
}