#include <Runtime/PAL/Sync/Thread.h>

#include <Runtime/Definitions/Allocator.h>

#include <Windows.h>
#include <process.h>

#include <string>
#include <utility>
#include <algorithm>

namespace Horizon::PAL
{
	namespace
	{
		struct ThreadStartContext
		{
			ThreadEntry entry;
			CustomUserData userData;
			std::string name;
		};

		void SetCurrentThreadName(const std::string& name)
		{
			if (name.empty())
				return;

			using SetThreadDescriptionFn = HRESULT(WINAPI*)(HANDLE, PCWSTR);

			static SetThreadDescriptionFn pSetThreadDescription = []() -> SetThreadDescriptionFn
				{
					HMODULE kernel = ::GetModuleHandleW(L"kernel32.dll");
					return kernel ? (SetThreadDescriptionFn)(
						::GetProcAddress(kernel, "SetThreadDescription")) : nullptr;
				}();

			if (!pSetThreadDescription)
				return;

			i32 wlen = ::MultiByteToWideChar(CP_UTF8, 0, name.data(), (i32)(name.size()), nullptr, 0);
			if (wlen <= 0)
				return;

			std::wstring wname((usize)(wlen), L'\0');
			::MultiByteToWideChar(CP_UTF8, 0, name.data(), (i32)(name.size()), wname.data(), wlen);
			pSetThreadDescription(::GetCurrentThread(), wname.c_str());
		}

		u32 __stdcall ThreadTrampoline(void* arg)
		{
			ThreadStartContext local = std::move(*(ThreadStartContext*)(arg));
			Memory::Allocator::Delete((ThreadStartContext*)(arg));

			SetCurrentThreadName(local.name);

			if (local.entry)
				local.entry(local.userData);

			return 0;
		}
	}

	Thread::Thread(ThreadEntry entry, CustomUserData userData, std::string_view name)
	{
		ThreadStartContext* ctx = Memory::Allocator::Create<ThreadStartContext>(Memory::CurrLoc(), entry, userData, std::string(name));

		uintptr_t h = ::_beginthreadex(nullptr, 0, &ThreadTrampoline, ctx, 0, nullptr);
		if (h == 0)
		{
			delete ctx;
			m_handle = nullptr;
			m_id = 0;
			return;
		}

		m_handle = (ThreadHandle)(h);
		m_id = (u64)(::GetThreadId((HANDLE)(h)));
	}

	Thread::~Thread()
	{
		if (m_handle)
			::CloseHandle((HANDLE)(m_handle));
	}

	Thread::Thread(Thread&& other) noexcept
		: m_handle(other.m_handle)
		, m_id(other.m_id)
	{
		other.m_handle = nullptr;
		other.m_id = 0;
	}

	Thread& Thread::operator=(Thread&& other) noexcept
	{
		if (this != &other)
		{
			if (m_handle)
				::CloseHandle((HANDLE)(m_handle));

			m_handle = other.m_handle;
			m_id = other.m_id;

			other.m_handle = nullptr;
			other.m_id = 0;
		}
		return *this;
	}

	void Thread::Join()
	{
		if (!m_handle)
			return;

		::WaitForSingleObject((HANDLE)(m_handle), INFINITE);
		::CloseHandle((HANDLE)(m_handle));

		m_handle = nullptr;
		m_id = 0;
	}

	void Thread::Detach()
	{
		if (!m_handle)
			return;

		::CloseHandle((HANDLE)(m_handle));

		m_handle = nullptr;
		m_id = 0;
	}

	void Thread::SetAffinity(u64 coreMask) const
	{
		if (m_handle)
			::SetThreadAffinityMask((HANDLE)(m_handle),
				(DWORD_PTR)(coreMask));
	}

	void Thread::SleepMs(u32 millisec)
	{
		::Sleep((DWORD)(millisec));
	}

	void Thread::YieldCurrent()
	{
		if (!::SwitchToThread())
			::Sleep(0);
	}

	u32 Thread::HardwareConcurrency()
	{
		SYSTEM_INFO info;
		::GetSystemInfo(&info);
		return (u32)(info.dwNumberOfProcessors);
	}

	u64 Thread::CurrentId()
	{
		return (u64)(::GetCurrentThreadId());
	}

	void Thread::SetCurrentAffinity(u64 coreMask)
	{
		if (coreMask == 0)
			return;

		::SetThreadAffinityMask(::GetCurrentThread(), (DWORD_PTR)(coreMask));
	}

	void Thread::SetCurrentPriority(ThreadPriority priority)
	{
		i32 native = THREAD_PRIORITY_NORMAL;

		switch (priority)
		{
		case ThreadPriority::Low:
			native = THREAD_PRIORITY_BELOW_NORMAL;
			break;
		case ThreadPriority::Normal:
			native = THREAD_PRIORITY_NORMAL;
			break;
		case ThreadPriority::High:
			native = THREAD_PRIORITY_ABOVE_NORMAL;
			break;
		case ThreadPriority::Highest:
			native = THREAD_PRIORITY_HIGHEST;
			break;
		case ThreadPriority::Realtime:
			native = THREAD_PRIORITY_TIME_CRITICAL;
			break;
		}

		::SetThreadPriority(::GetCurrentThread(), native);
	}

	void Thread::SetCurrentEcoQoS(b8 enabled)
	{
		THREAD_POWER_THROTTLING_STATE state = {};
		state.Version = THREAD_POWER_THROTTLING_CURRENT_VERSION;
		state.ControlMask = THREAD_POWER_THROTTLING_EXECUTION_SPEED;
		state.StateMask = enabled ? THREAD_POWER_THROTTLING_EXECUTION_SPEED : 0;

		::SetThreadInformation(::GetCurrentThread(), ThreadPowerThrottling, &state, sizeof(state));
	}
}