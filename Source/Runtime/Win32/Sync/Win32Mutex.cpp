#include <Runtime/PAL/Sync/Mutex.h>

#include <Windows.h>

namespace Horizon::PAL
{
	Mutex::Mutex()
	{
		m_handle = (MutexHandle)(::CreateMutexW(nullptr, FALSE, nullptr));
	}

	Mutex::~Mutex()
	{
		if (m_handle)
			::CloseHandle((HANDLE)(m_handle));
	}

	Mutex::Mutex(Mutex&& other) noexcept : m_handle(other.m_handle)
	{
		other.m_handle = nullptr;
	}

	Mutex& Mutex::operator=(Mutex&& other) noexcept
	{
		if (this != &other)
		{
			if (m_handle)
				::CloseHandle((HANDLE)(m_handle));

			m_handle = other.m_handle;
			other.m_handle = nullptr;
		}
		return *this;
	}

	void Mutex::Lock() const
	{
		::WaitForSingleObject((HANDLE)(m_handle), INFINITE);
	}

	void Mutex::Unlock() const
	{
		::ReleaseMutex((HANDLE)(m_handle));
	}

	b8 Mutex::TryLock() const
	{
		return ::WaitForSingleObject((HANDLE)(m_handle), 0) == WAIT_OBJECT_0;
	}
}