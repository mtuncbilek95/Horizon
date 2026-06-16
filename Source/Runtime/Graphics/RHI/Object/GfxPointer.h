#pragma once

namespace Horizon
{
	template<typename T>
	class GfxPointer
	{
	public:
		GfxPointer() = default;
		GfxPointer(T* pPtr) : m_ptr(pPtr) {}
		GfxPointer(const GfxPointer& other) : m_ptr(other.m_ptr)
		{
			if (m_ptr)
				m_ptr->IncrementPointer();
		}

		GfxPointer(GfxPointer&& other) noexcept : m_ptr(other.m_ptr)
		{
			other.m_ptr = nullptr;
		}

		~GfxPointer()
		{
			if (m_ptr)
				m_ptr->Release();
		}

		GfxPointer& operator=(GfxPointer other) { std::swap(m_ptr, other.m_ptr); return *this; }

		T* operator->() const { return m_ptr; }
		T* GetRawPtr() const { return m_ptr; }

	private:
		T* m_ptr = nullptr;
	};
}