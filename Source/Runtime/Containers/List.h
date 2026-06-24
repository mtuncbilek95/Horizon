#pragma once

#include <initializer_list>
#include <memory>

namespace Horizon
{
	template<typename T>
	class H_EXPORT List final
	{
	public:
		using Iterator = T*;
		using ConstIterator = const T*;
		using ReverseIterator = std::reverse_iterator<Iterator>;
		using ConstReverseIterator = std::reverse_iterator<ConstIterator>;

	public:
		constexpr List() noexcept : m_data(nullptr)
		{
			m_capacity = 0;
			m_size = 0;
		}

		constexpr List(u64 size) : m_data(AllocateBuffer(size))
		{
			m_capacity = size;
			m_size = size;
			for (u64 i = 0; i < m_size; i++)
				new (m_data + i) T();
		}

		constexpr List(u64 size, const T& defaultValue) : m_data(AllocateBuffer(size))
		{
			m_capacity = size;
			m_size = size;
			InitializeElements(defaultValue);
		}

		constexpr List(const std::initializer_list<T> initList) : m_data(AllocateBuffer(initList.size()))
		{
			m_size = initList.size();
			m_capacity = initList.size();

			CopyFrom(initList.begin(), initList.size());
		}

		constexpr List(const List& other) : m_data(AllocateBuffer(other.Capacity()))
		{
			m_size = other.Size();
			m_capacity = other.Capacity();

			CopyFrom(other.begin(), other.Size());
		}

		constexpr List(List&& mover) noexcept : m_data(nullptr)
		{
			m_size = 0;
			m_capacity = 0;
			Swap(mover);
		}

		constexpr List& operator=(const List& other)
		{
			if (&other == this)
				return *this;

			List temp(other);
			Swap(temp);

			return *this;
		}

		constexpr List& operator=(List&& move) noexcept
		{
			if (&move == this)
				return *this;

			Swap(move);
			return *this;
		}

		constexpr ~List()
		{
			DestroyElements();

			if (m_data)
				::operator delete(m_data);
		}

		constexpr b8 Has(const T& element) const
		{
			for (u64 i = 0; i < m_size; i++)
			{
				if (m_data[i] == element)
					return true;
			}

			return false;
		}

		constexpr void PushBack(const T& value)
		{
			ResizeIfRequired();

			new (m_data + m_size) T(value);
			++m_size;
		}

		constexpr void PushAt(const T& value, u64 index)
		{
			ResizeIfRequired();

			if (index == m_size)
				new (m_data + m_size) T(value);
			else
			{
				new (m_data + m_size) T(std::move(m_data[m_size - 1]));

				for (u64 i = m_size - 1; i > index; --i)
					m_data[i] = std::move(m_data[i - 1]);

				m_data[index] = value;
			}
			m_size++;
		}

		constexpr i64 FindIndex(const T& element) const
		{
			for (u64 i = 0; i < m_size; i++)
			{
				if (m_data[i] == element)
					return i;
			}

			return -1;
		}

		template<typename TOther>
			requires std::equality_comparable_with<T, TOther> // If can check on compile-time, let it check it.
		constexpr i64 FindIndex(const TOther& element) const
		{
			for (u64 i = 0; i < m_size; i++)
			{
				if (m_data[i] == element)
					return i;
			}

			return -1;
		}

		template<typename Compare>
		void Sort(Compare comp)
		{
			if (m_size < 2)
				return;

			QuickSortInternal(0, i64(m_size - 1), comp);
		}

		constexpr b8 Remove(const T& element)
		{
			const i64 index = FindIndex(element);
			if (index == -1)
				return false;

			m_data[index].~T();

			for (u64 i = index + 1; i < m_size; i++)
				m_data[i - 1] = std::move(m_data[i]);

			m_size--;
			return true;
		}

		template<typename TOther>
			requires std::equality_comparable_with<T, TOther> // If can check on compile-time, let it check it.
		constexpr b8 Remove(const TOther& element)
		{
			const i64 index = FindIndex(element);
			if (index == -1)
				return false;

			RemoveAt(static_cast<u64>(index));
			return true;
		}

		constexpr void RemoveLast()
		{
			if (m_size > 0)
			{
				m_size--;
				m_data[m_size].~T();
			}
		}

		constexpr void RemoveFirst()
		{
			if (m_size > 0)
				RemoveAt(0);
		}

		constexpr void RemoveAt(u64 index)
		{
			if (index >= m_size)
				return;

			for (u64 i = index; i < m_size - 1; ++i)
				m_data[i] = std::move(m_data[i + 1]);

			m_data[m_size - 1].~T();
			m_size--;
		}

		constexpr void Free()
		{
			DestroyElements();
			m_size = 0;
		}

		constexpr void Reset()
		{
			m_size = 0;
		}

		constexpr void Reserve(u64 capacity)
		{
			if (capacity > m_capacity)
				Reallocate(capacity);
		}

		constexpr void Resize(u64 size)
		{
			if (size > m_capacity)
				Reallocate(size);

			AdjustSize(size);
		}

		constexpr void Swap(List& other) noexcept
		{
			using std::swap;
			swap(m_capacity, other.m_capacity);
			swap(m_size, other.m_size);
			swap(m_data, other.m_data);
		}

		constexpr u64 Capacity() const { return m_capacity; }
		constexpr u64 Size() const { return m_size; }

		constexpr T& At(u64 index) { return m_data[index]; }
		constexpr const T& At(u64 index) const { return m_data[index]; }

		constexpr T* Data() { return m_data; }
		constexpr const T* Data() const { return m_data; }

		constexpr T& operator[](u64 index) { return m_data[index]; }
		constexpr const T& operator[](u64 index) const { return m_data[index]; }

		constexpr T& Last() { return m_data[m_size - 1]; }
		constexpr const T& Last() const { return m_data[m_size - 1]; }

		constexpr T& First() { return m_data[0]; }
		constexpr const T& First() const { return m_data[0]; }

		FORCEINLINE b8 IsEmpty() const { return m_size == 0; }
		FORCEINLINE b8 IsValid() const { return m_data != nullptr; }

		constexpr b8 operator==(const List& other) const
		{
			if (m_size != other.m_size)
				return false;

			for (u64 i = 0; i < m_size; i++)
			{
				if (m_data[i] != other.m_data[i])
					return false;
			}

			return true;
		}

		Iterator begin() { return Iterator(m_data); }
		Iterator end() { return Iterator(m_data + m_size); }

		ConstIterator begin() const { return ConstIterator(m_data); }
		ConstIterator end() const { return ConstIterator(m_data + m_size); }
		ConstIterator cbegin() const { return ConstIterator(m_data); }
		ConstIterator cend() const { return ConstIterator(m_data + m_size); }

		ReverseIterator rbegin() { return ReverseIterator(end()); }
		ReverseIterator rend() { return ReverseIterator(begin()); }

		ConstReverseIterator rbegin() const { return ConstReverseIterator(end()); }
		ConstReverseIterator rend() const { return ConstReverseIterator(begin()); }

		ConstReverseIterator crbegin() const { return ConstReverseIterator(end()); }
		ConstReverseIterator crend() const { return ConstReverseIterator(begin()); }

	private:
		template <typename Compare>
		void QuickSortInternal(i64 left, i64 right, Compare comp)
		{
			i64 i = left, j = right;

			T pivot = m_data[(left + right) / 2];

			while (i <= j)
			{
				while (comp(m_data[i], pivot))
					i++;

				while (comp(pivot, m_data[j]))
					j--;

				if (i <= j)
				{
					T temp = m_data[i];
					m_data[i] = m_data[j];
					m_data[j] = temp;
					i++;
					j--;
				}
			}

			if (left < j)
				QuickSortInternal(left, j, comp);

			if (i < right)
				QuickSortInternal(i, right, comp);
		}

		constexpr T* AllocateBuffer(u64 capacity)
		{
			if (!capacity)
				return nullptr;

			return static_cast<T*>(::operator new(sizeof(T) * capacity));
		}

		constexpr void InitializeElements(const T& value)
		{
			for (u64 i = 0; i < m_size; ++i)
				new (m_data + i) T(value);
		}

		constexpr void ResizeIfRequired(i64 index = -1)
		{
			// If index is bigger than the cap, create a proper 2^n cap 
			if (index != -1 && index > m_capacity)
			{
				u64 newCap = m_capacity;
				while (index > m_capacity)
					newCap *= 2;

				Reallocate(newCap);
			}

			if (m_size == m_capacity)
				Reallocate(m_capacity == 0 ? 1 : m_capacity * 2);
		}

		constexpr void CopyFrom(const T* source, u64 count)
		{
			for (u64 i = 0; i < count; ++i)
				new (m_data + i) T(source[i]);

			m_size = count;
		}

		constexpr void DestroyElements()
		{
			for (u64 i = 0; i < m_size; ++i)
				m_data[i].~T();
		}

		constexpr void AdjustSize(u64 newSize)
		{
			if (newSize > m_size)
			{
				for (u64 i = m_size; i < newSize; ++i)
					new (m_data + i) T();
			}
			else if (newSize < m_size)
			{
				for (u64 i = newSize; i < m_size; ++i)
					m_data[i].~T();
			}

			m_size = newSize;
		}

		constexpr void ShiftRight(u64 index)
		{
			new (m_data + m_size) T(std::move(m_data[m_size - 1]));

			for (u64 i = m_size - 1; i > index; --i)
				m_data[i] = std::move(m_data[i - 1]);
		}

		constexpr void Reallocate(u64 newCapacity)
		{
			if (newCapacity <= m_capacity && m_data)
				return;

			T* newBuffer = AllocateBuffer(newCapacity);

			for (u64 i = 0; i < m_size; ++i)
			{
				new (newBuffer + i) T(std::move(m_data[i]));
				m_data[i].~T();
			}

			if (m_data)
				::operator delete(m_data);

			m_data = newBuffer;
			m_capacity = newCapacity;
		}

	private:
		T* m_data;
		u64 m_capacity;
		u64 m_size;
	};
}