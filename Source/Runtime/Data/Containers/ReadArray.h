#pragma once

#include <cstdint>
#include <cstring>
#include <iterator>
#include <span>

namespace Horizon 
{
	template<typename T = u32>
	class ReadArray 
	{
	public:
		template<typename It>
		ReadArray(It begin, It end) 
		{
			m_size = static_cast<u32>(std::distance(begin, end));
			m_data = static_cast<T*>(::operator new(m_size * sizeof(T)));

			T* dst = m_data;

			for (It it = begin; it != end; ++it, ++dst)
				new (dst) T(*it);
		}

		ReadArray(const T* ptr, u32 count) : ReadArray(ptr, ptr + count) {}

		~ReadArray() 
		{
			for (u32 i = 0; i < m_size; ++i)
				m_data[i].~T();
			::operator delete(m_data);
		}

		ReadArray(const ReadArray&) = delete;
		ReadArray& operator=(const ReadArray&) = delete;

		ReadArray(ReadArray&& other) noexcept : m_data(other.m_data), 
			m_size(other.m_size) 
		{
			other.m_data = nullptr;
			other.m_size = 0;
		}
		ReadArray& operator=(ReadArray&&) = delete;

		std::span<T> operator*()   const { return { m_data, m_size }; }
		const T& operator[](u32 i) const { return m_data[i]; }

		T* Data() const { return m_data; }
		u32 Size() const { return m_size; }

	private:
		T* m_data = nullptr;
		u32 m_size = 0;
	};

}
