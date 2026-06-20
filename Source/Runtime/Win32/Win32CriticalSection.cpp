#include <Runtime/PAL/Sync/CriticalSection.h>

#include <Windows.h>

namespace Horizon
{
	CriticalSection::CriticalSection()
	{
		InitializeCriticalSection(LPCRITICAL_SECTION(m_handle));
	}
	
	CriticalSection::~CriticalSection()
	{
		DeleteCriticalSection(LPCRITICAL_SECTION(m_handle));
	}

	b8 CriticalSection::TryLock() const
	{
		return TryEnterCriticalSection(LPCRITICAL_SECTION(m_handle));
	}

	void CriticalSection::Lock() const
	{
		EnterCriticalSection(LPCRITICAL_SECTION(m_handle));
	}

	void CriticalSection::Unlock() const
	{
		LeaveCriticalSection(LPCRITICAL_SECTION(m_handle));
	}
}