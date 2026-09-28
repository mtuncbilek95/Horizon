#pragma once

#include <Runtime/Containers/List.h>

#include <iterator>
#include <type_traits>

namespace Horizon
{
	template<typename T>
	class ReadOnlyList
	{
	public:
		using ConstIterator = const T*;
		using ConstReverseIterator = std::reverse_iterator<ConstIterator>;

	public:
		ReadOnlyList() = default;

		ReadOnlyList(const List<std::remove_const_t<T>>& list) : m_data(list.GetData()), m_count(list.GetCount())
		{
		}

		ReadOnlyList(const T* pData, usize count) : m_data(pData), m_count(count)
		{
		}

		usize GetCount() const { return m_count; }
		const T* GetData() const { return m_data; }

		const T& At(usize index) const { return m_data[index]; }
		const T& operator[](usize index) const { return m_data[index]; }

		const T& Front() const { return m_data[0]; }
		const T& Back() const { return m_data[m_count - 1]; }

		b8 IsEmpty() const { return m_count == 0; }
		b8 IsValid() const { return m_data != nullptr; }

		ConstIterator begin() const { return m_data; }
		ConstIterator end() const { return m_data + m_count; }
		ConstIterator cbegin() const { return m_data; }
		ConstIterator cend() const { return m_data + m_count; }

		ConstReverseIterator rbegin() const { return ConstReverseIterator(end()); }
		ConstReverseIterator rend() const { return ConstReverseIterator(begin()); }

	private:
		const T* m_data = nullptr;
		usize m_count = 0;
	};
}