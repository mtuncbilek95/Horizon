#pragma once

#include <Runtime/Containers/ListBase.h>
#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>

#include <initializer_list>
#include <iterator>
#include <concepts>
#include <utility>

namespace Horizon
{
	template<typename T>
	class List final : public ListBase
	{
	public:
		using Iterator = T*;
		using ConstIterator = const T*;
		using ReverseIterator = std::reverse_iterator<Iterator>;
		using ConstReverseIterator = std::reverse_iterator<ConstIterator>;

	public:
		List() = default;

		explicit List(usize count)
		{
			m_data = AllocateBuffer(count);
			m_count = count;
			m_capacity = count;

			for (usize i = 0; i < count; i++)
				::new (m_data + i) T();
		}

		List(usize count, const T& value)
		{
			m_data = AllocateBuffer(count);
			m_count = count;
			m_capacity = count;

			for (usize i = 0; i < count; i++)
				::new (m_data + i) T(value);
		}

		List(std::initializer_list<T> initList)
		{
			m_data = AllocateBuffer(initList.size());
			m_capacity = initList.size();

			CopyFrom(initList.begin(), initList.size());
		}

		template<std::forward_iterator TIterator>
			requires std::constructible_from<T, std::iter_reference_t<TIterator>>
		List(TIterator first, TIterator last)
		{
			const usize count = static_cast<usize>(std::distance(first, last));

			m_data = AllocateBuffer(count);
			m_capacity = count;

			for (usize i = 0; i < count; ++i)
			{
				::new (m_data + i) T(*first);
				++first;
			}

			m_count = count;
		}

		List(const List& other)
		{
			m_data = AllocateBuffer(other.m_count);
			m_capacity = other.m_count;

			CopyFrom(other.m_data, other.m_count);
		}

		List(List&& other) noexcept
		{
			Swap(other);
		}

		List& operator=(const List& other)
		{
			if (&other == this)
				return *this;

			List temp(other);

			Swap(temp);
			return *this;
		}

		List& operator=(List&& other) noexcept
		{
			if (&other == this)
				return *this;

			Swap(other);
			return *this;
		}

		~List() override
		{
			DestroyElements();

			if (m_data)
				FreeBuffer(m_data);
		}

		usize GetElementSize() const override { return sizeof(T); }
		usize GetCount() const override { return m_count; }

		void* GetElementAt(usize index) override { return m_data + index; }
		const void* GetElementAt(usize index) const override { return m_data + index; }

		usize GetCapacity() const { return m_capacity; }

		T* GetData() { return m_data; }
		const T* GetData() const { return m_data; }

		T& At(usize index) { return m_data[index]; }
		const T& At(usize index) const { return m_data[index]; }

		T& operator[](usize index) { return m_data[index]; }
		const T& operator[](usize index) const { return m_data[index]; }

		T& Front() { return m_data[0]; }
		const T& Front() const { return m_data[0]; }

		T& Back() { return m_data[m_count - 1]; }
		const T& Back() const { return m_data[m_count - 1]; }

		b8 IsEmpty() const { return m_count == 0; }
		b8 IsValid() const { return m_data != nullptr; }

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
			if (m_count == m_capacity)
			{
				const usize newCapacity = m_capacity == 0 ? 2 : m_capacity * 2;
				T* pNewBuffer = AllocateBuffer(newCapacity);

				::new (pNewBuffer + m_count) T(std::forward<Args>(args)...);

				RelocateInto(pNewBuffer);

				if (m_data)
					FreeBuffer(m_data);

				m_data = pNewBuffer;
				m_capacity = newCapacity;
				m_count++;

				return Back();
			}

			::new (m_data + m_count) T(std::forward<Args>(args)...);
			m_count++;

			return Back();
		}

		void PushAt(usize index, const T& value)
		{
			EmplaceAt(index, value);
		}

		void PushAt(usize index, T&& value)
		{
			EmplaceAt(index, std::move(value));
		}

		template<typename... Args>
		T& EmplaceAt(usize index, Args&&... args)
		{
			if (index >= m_count)
				return EmplaceBack(std::forward<Args>(args)...);

			GrowIfRequired();

			::new (m_data + m_count) T(std::move(m_data[m_count - 1]));

			for (usize i = m_count - 1; i > index; --i)
				m_data[i] = std::move(m_data[i - 1]);

			m_data[index] = T(std::forward<Args>(args)...);
			m_count++;

			return m_data[index];
		}

		b8 Contains(const T& element) const
		{
			return FindIndex(element) != -1;
		}

		i64 FindIndex(const T& element) const
		{
			for (usize i = 0; i < m_count; i++)
			{
				if (m_data[i] == element)
					return static_cast<i64>(i);
			}

			return -1;
		}

		template<typename TOther>
			requires std::equality_comparable_with<T, TOther>
		i64 FindIndex(const TOther& element) const
		{
			for (usize i = 0; i < m_count; i++)
			{
				if (m_data[i] == element)
					return static_cast<i64>(i);
			}

			return -1;
		}

		b8 Remove(const T& element)
		{
			const i64 index = FindIndex(element);

			if (index == -1)
				return false;

			RemoveAt(static_cast<usize>(index));
			return true;
		}

		template<typename TOther>
			requires std::equality_comparable_with<T, TOther>
		b8 Remove(const TOther& element)
		{
			const i64 index = FindIndex(element);

			if (index == -1)
				return false;

			RemoveAt(static_cast<usize>(index));
			return true;
		}

		void RemoveAt(usize index) override
		{
			if (index >= m_count)
				return;

			if constexpr (!std::is_move_assignable_v<T>)
			{
				Terminal::Error(StringOps::GetName(this), "Element type is not move assignable, RemoveAt({}) ignored", index);
				return;
			}
			else
			{
				for (usize i = index; i < m_count - 1; ++i)
					m_data[i] = std::move(m_data[i + 1]);

				m_data[m_count - 1].~T();
				m_count--;
			}
		}

		void PopBack()
		{
			if (m_count == 0)
				return;

			m_count--;
			m_data[m_count].~T();
		}

		void PopFront()
		{
			if (m_count > 0)
				RemoveAt(0);
		}

		void Clear()
		{
			DestroyElements();
			m_count = 0;
		}

		void Reserve(usize capacity)
		{
			if (capacity > m_capacity)
				Reallocate(capacity);
		}

		void Resize(usize count) override
		{
			if constexpr (!std::is_default_constructible_v<T>)
			{
				if (count > m_count)
				{
					Terminal::Error(StringOps::GetName(this), "Element type is not default constructible, resize to {} ignored", count);
					return;
				}
			}

			if (count > m_capacity)
				Reallocate(count);

			AdjustSize(count);
		}

		void Swap(List& other) noexcept
		{
			std::swap(m_data, other.m_data);
			std::swap(m_count, other.m_count);
			std::swap(m_capacity, other.m_capacity);
		}

		template<typename Compare>
		void Sort(Compare comp)
		{
			if (m_count < 2)
				return;

			u32 depth = 0;
			for (usize n = m_count; n > 1; n >>= 1)
				depth++;

			QuickSortInternal(0, static_cast<i64>(m_count - 1), comp, depth * 2);
		}

		void ReverseOrder()
		{
			if (m_count < 2)
				return;

			for (usize i = 0, j = m_count - 1; i < j; ++i, --j)
				std::swap(m_data[i], m_data[j]);
		}

		b8 operator==(const List& other) const
		{
			if (m_count != other.m_count)
				return false;

			for (usize i = 0; i < m_count; i++)
			{
				if (m_data[i] != other.m_data[i])
					return false;
			}

			return true;
		}

		Iterator begin() { return m_data; }
		Iterator end() { return m_data + m_count; }

		ConstIterator begin() const { return m_data; }
		ConstIterator end() const { return m_data + m_count; }
		ConstIterator cbegin() const { return m_data; }
		ConstIterator cend() const { return m_data + m_count; }

		ReverseIterator rbegin() { return ReverseIterator(end()); }
		ReverseIterator rend() { return ReverseIterator(begin()); }

		ConstReverseIterator rbegin() const { return ConstReverseIterator(end()); }
		ConstReverseIterator rend() const { return ConstReverseIterator(begin()); }
		ConstReverseIterator crbegin() const { return ConstReverseIterator(cend()); }
		ConstReverseIterator crend() const { return ConstReverseIterator(cbegin()); }

	private:
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

		void CopyFrom(const T* pSource, usize count)
		{
			for (usize i = 0; i < count; ++i)
				::new (m_data + i) T(pSource[i]);

			m_count = count;
		}

		void DestroyElements()
		{
			for (usize i = 0; i < m_count; ++i)
				m_data[i].~T();
		}

		void GrowIfRequired()
		{
			if (m_count == m_capacity)
				Reallocate(m_capacity == 0 ? 2 : m_capacity * 2);
		}

		void AdjustSize(usize newCount)
		{
			if (newCount > m_count)
			{
				if constexpr (std::is_default_constructible_v<T>)
				{
					for (usize i = m_count; i < newCount; ++i)
						::new (m_data + i) T();
				}
			}
			else if (newCount < m_count)
			{
				for (usize i = newCount; i < m_count; ++i)
					m_data[i].~T();
			}

			m_count = newCount;
		}

		void RelocateInto(T* pDestination)
		{
			if constexpr (std::is_trivially_copyable_v<T>)
			{
				if (m_count > 0)
					std::memcpy(pDestination, m_data, sizeof(T) * m_count);

				return;
			}
			else
			{
				for (usize i = 0; i < m_count; ++i)
				{
					::new (pDestination + i) T(std::move(m_data[i]));
					m_data[i].~T();
				}
			}
		}

		void Reallocate(usize newCapacity)
		{
			T* pNewBuffer = AllocateBuffer(newCapacity);

			RelocateInto(pNewBuffer);

			if (m_data)
				FreeBuffer(m_data);

			m_data = pNewBuffer;
			m_capacity = newCapacity;
		}

		template<typename Compare>
		void QuickSortInternal(i64 left, i64 right, Compare comp, u32 depth)
		{
			while (left < right)
			{
				if (right - left < 16)
				{
					InsertionSortInternal(left, right, comp);
					return;
				}

				if (depth == 0)
				{
					HeapSortInternal(left, right, comp);
					return;
				}

				const i64 mid = left + (right - left) / 2;
				std::swap(m_data[mid], m_data[right]);

				i64 store = left;

				for (i64 i = left; i < right; ++i)
				{
					if (comp(m_data[i], m_data[right]))
					{
						std::swap(m_data[i], m_data[store]);
						store++;
					}
				}

				std::swap(m_data[store], m_data[right]);

				depth--;

				if (store - left < right - store)
				{
					QuickSortInternal(left, store - 1, comp, depth);
					left = store + 1;
				}
				else
				{
					QuickSortInternal(store + 1, right, comp, depth);
					right = store - 1;
				}
			}
		}

		template<typename Compare>
		void InsertionSortInternal(i64 left, i64 right, Compare comp)
		{
			for (i64 i = left + 1; i <= right; ++i)
			{
				T value = std::move(m_data[i]);
				i64 j = i - 1;

				while (j >= left && comp(value, m_data[j]))
				{
					m_data[j + 1] = std::move(m_data[j]);
					j--;
				}

				m_data[j + 1] = std::move(value);
			}
		}

		template<typename Compare>
		void SiftDownInternal(i64 left, i64 root, i64 end, Compare comp)
		{
			while (true)
			{
				i64 child = 2 * (root - left) + 1 + left;

				if (child > end)
					return;

				if (child + 1 <= end && comp(m_data[child], m_data[child + 1]))
					child++;

				if (!comp(m_data[root], m_data[child]))
					return;

				std::swap(m_data[root], m_data[child]);
				root = child;
			}
		}

		template<typename Compare>
		void HeapSortInternal(i64 left, i64 right, Compare comp)
		{
			const i64 count = right - left + 1;

			if (count < 2)
				return;

			for (i64 root = left + count / 2 - 1; root >= left; --root)
				SiftDownInternal(left, root, right, comp);

			for (i64 end = right; end > left; --end)
			{
				std::swap(m_data[left], m_data[end]);
				SiftDownInternal(left, left, end - 1, comp);
			}
		}

	private:
		T* m_data = nullptr;
		usize m_count = 0;
		usize m_capacity = 0;
	};
}