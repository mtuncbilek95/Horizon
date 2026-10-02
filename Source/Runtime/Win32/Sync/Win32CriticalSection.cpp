#include <Runtime/PAL/Sync/CriticalSection.h>

#include <Runtime/Definitions/Allocator.h>

#include <Windows.h>

namespace Horizon::PAL
{
	CriticalSection::CriticalSection()
	{
		m_handle = Memory::Allocator::Create<CRITICAL_SECTION>(Memory::CurrLoc());
		InitializeCriticalSection(LPCRITICAL_SECTION(m_handle));
	}
	
	CriticalSection::~CriticalSection()
	{
		DeleteCriticalSection(LPCRITICAL_SECTION(m_handle));
		Memory::Allocator::Delete((CRITICAL_SECTION*)(m_handle));
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