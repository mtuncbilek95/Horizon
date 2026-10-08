#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <iterator>
#include <new>
#include <utility>

namespace Horizon
{
	template<typename T>
	class RingBuffer final
	{
	public:
		template<typename TValue, typename TOwner>
		class IteratorBase
		{
		public:
			using iterator_category = std::forward_iterator_tag;
			using value_type = TValue;
			using difference_type = i64;
			using pointer = TValue*;
			using reference = TValue&;

			IteratorBase() = default;
			IteratorBase(TOwner* pOwner, usize logicalIndex) : m_owner(pOwner), m_index(logicalIndex) {}

			reference operator*() const { return m_owner->At(m_index); }
			pointer operator->() const { return &m_owner->At(m_index); }

			IteratorBase& operator++()
			{
				++m_index;
				return *this;
			}

			IteratorBase operator++(int)
			{
				IteratorBase copy = *this;
				++m_index;
				return copy;
			}

			b8 operator==(const IteratorBase& other) const { return m_owner == other.m_owner && m_index == other.m_index; }
			b8 operator!=(const IteratorBase& other) const { return !(*this == other); }

		private:
			TOwner* m_owner = nullptr;
			usize m_index = 0;
		};

		using Iterator = IteratorBase<T, RingBuffer>;
		using ConstIterator = IteratorBase<const T, const RingBuffer>;

	public:
		RingBuffer() = default;

		explicit RingBuffer(usize capacity)
		{
			m_data = AllocateBuffer(capacity);
			m_capacity = capacity;
		}

		RingBuffer(const RingBuffer&) = delete;
		RingBuffer& operator=(const RingBuffer&) = delete;

		RingBuffer(RingBuffer&& other) noexcept
		{
			Swap(other);
		}

		RingBuffer& operator=(RingBuffer&& other) noexcept
		{
			if (this == &other)
				return *this;

			Swap(other);
			return *this;
		}

		~RingBuffer()
		{
			DestroyElements();

			if (m_data)
				FreeBuffer(m_data);
		}

		usize GetCount() const { return m_count; }
		usize GetCapacity() const { return m_capacity; }

		b8 IsEmpty() const { return m_count == 0; }
		b8 IsFull() const { return m_count == m_capacity; }
		b8 IsValid() const { return m_data != nullptr; }

		T& At(usize logicalIndex) { return m_data[PhysicalIndex(logicalIndex)]; }
		const T& At(usize logicalIndex) const { return m_data[PhysicalIndex(logicalIndex)]; }

		T& operator[](usize logicalIndex) { return At(logicalIndex); }
		const T& operator[](usize logicalIndex) const { return At(logicalIndex); }

		T& Front() { return At(0); }
		const T& Front() const { return At(0); }

		T& Back() { return At(m_count - 1); }
		const T& Back() const { return At(m_count - 1); }

		void PushBack(const T& value)
		{
			EmplaceBack(value);
		}

		void PushBack(T&& value)
		{
			EmplaceBack(std::move(value));
		}

		template<typename... Args>
		T& EmplaceBack(Args&&... args)
		{
			if (m_capacity == 0)
				return *static_cast<T*>(nullptr);

			if (IsFull())
			{
				T* pOldest = &m_data[m_head];
				pOldest->~T();
				::new (pOldest) T(std::forward<Args>(args)...);
				m_head = (m_head + 1) % m_capacity;
				return *pOldest;
			}

			T* pSlot = &m_data[(m_head + m_count) % m_capacity];
			::new (pSlot) T(std::forward<Args>(args)...);
			m_count++;
			return *pSlot;
		}

		void PopFront()
		{
			if (m_count == 0)
				return;

			m_data[m_head].~T();
			m_head = (m_head + 1) % m_capacity;
			m_count--;
		}

		void Clear()
		{
			DestroyElements();
			m_head = 0;
		}

		void Reset(usize capacity)
		{
			DestroyElements();

			if (m_data)
				FreeBuffer(m_data);

			m_data = AllocateBuffer(capacity);
			m_capacity = capacity;
			m_head = 0;
		}

		void Swap(RingBuffer& other) noexcept
		{
			std::swap(m_data, other.m_data);
			std::swap(m_capacity, other.m_capacity);
			std::swap(m_count, other.m_count);
			std::swap(m_head, other.m_head);
		}

		Iterator begin() { return Iterator(this, 0); }
		Iterator end() { return Iterator(this, m_count); }

		ConstIterator begin() const { return ConstIterator(this, 0); }
		ConstIterator end() const { return ConstIterator(this, m_count); }
		ConstIterator cbegin() const { return ConstIterator(this, 0); }
		ConstIterator cend() const { return ConstIterator(this, m_count); }

	private:
		usize PhysicalIndex(usize logicalIndex) const
		{
			return (m_head + logicalIndex) % m_capacity;
		}

		void DestroyElements()
		{
			for (usize i = 0; i < m_count; ++i)
				m_data[PhysicalIndex(i)].~T();

			m_count = 0;
		}

		T* AllocateBuffer(usize capacity)
		{
			if (capacity == 0)
				return nullptr;

			return static_cast<T*>(::operator new(sizeof(T) * capacity, std::align_val_t{ alignof(T) }));
		}

		void FreeBuffer(T* pBuffer)
		{
			::operator delete(pBuffer, std::align_val_t{ alignof(T) });
		}

	private:
		T* m_data = nullptr;
		usize m_capacity = 0;
		usize m_count = 0;
		usize m_head = 0;
	};
}
