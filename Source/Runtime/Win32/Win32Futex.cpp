#include <Runtime/PAL/Sync/Futex.h>

#include <Windows.h>

namespace Horizon::PAL
{
	void Futex::Wait(void* pAddress, i64 expectedValue)
	{
		WaitOnAddress(pAddress, &expectedValue, sizeof(i64), INFINITE);
	}

	void Futex::WakeAll(void* pAddress)
	{
		WakeByAddressAll(pAddress);
	}

	void Futex::WakeSingle(void* pAddress)
	{
		WakeByAddressSingle(pAddress);
	}
}