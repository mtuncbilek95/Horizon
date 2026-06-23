#include <Runtime/PAL/Sync/Semaphore.h>

#include <Windows.h>

namespace Horizon
{
	Semaphore::Semaphore(u32 initialCount, u32 maxCount)
	{
		m_handle = (SemaphoreHandle)(::CreateSemaphoreW(
			nullptr,
			(LONG)(initialCount),
			(LONG)(maxCount),
			nullptr));
	}

	Semaphore::~Semaphore()
	{
		if (m_handle)
			::CloseHandle((HANDLE)(m_handle));
	}

	void Semaphore::Acquire() const
	{
		::WaitForSingleObject((HANDLE)(m_handle), INFINITE);
	}

	b8 Semaphore::TryAcquire(u32 timeoutMs) const
	{
		return ::WaitForSingleObject((HANDLE)(m_handle), (DWORD)(timeoutMs)) == WAIT_OBJECT_0;
	}

	void Semaphore::Release(u32 count) const
	{
		::ReleaseSemaphore((HANDLE)(m_handle), (LONG)(count), nullptr);
	}
}