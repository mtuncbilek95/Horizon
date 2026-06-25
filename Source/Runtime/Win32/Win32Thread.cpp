#include <Runtime/PAL/Sync/Thread.h>

#include <Windows.h>
#include <process.h>

#include <string>
#include <vector>
#include <utility>
#include <algorithm>

namespace Horizon
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
			Allocator::Delete((ThreadStartContext*)(arg));

			SetCurrentThreadName(local.name);

			if (local.entry)
				local.entry(local.userData);

			return 0;
		}
	}

	Thread::Thread(ThreadEntry entry, CustomUserData userData, std::string_view name)
	{
		ThreadStartContext* ctx = Allocator::Create<ThreadStartContext>(CurrLoc(), entry, userData, std::string(name));

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

	void Thread::SetAffinity(u64 coreMask)
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

	std::vector<CoreInfo> Thread::EnumerateCores()
	{
		std::vector<CoreInfo> result;

		DWORD len = 0;
		::GetLogicalProcessorInformationEx(RelationProcessorCore, nullptr, &len);
		if (len == 0)
			return result;

		std::vector<u8> buffer(len);
		if (!::GetLogicalProcessorInformationEx(RelationProcessorCore,
			(SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*)(buffer.data()), &len))
			return result;

		auto forEachCore = [&](auto&& fn)
			{
				DWORD offset = 0;
				while (offset < len)
				{
					auto* rec = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*)(buffer.data() + offset);
					if (rec->Relationship == RelationProcessorCore)
						fn(*rec);
					offset += rec->Size;
				}
			};

		BYTE maxClass = 0;
		forEachCore([&](const SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX& rec)
			{
				maxClass = (std::max)(maxClass, rec.Processor.EfficiencyClass);
			});

		forEachCore([&](const SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX& rec)
			{
				const b8 isPerf = (rec.Processor.EfficiencyClass == maxClass);

				if (rec.Processor.GroupCount == 0)
					return;

				const KAFFINITY mask = rec.Processor.GroupMask[0].Mask;
				for (u32 bit = 0; bit < sizeof(KAFFINITY) * 8; ++bit)
				{
					if (mask & (KAFFINITY(1) << bit))
					{
						result.push_back(CoreInfo{ bit, isPerf });
						break;
					}
				}
			});

		return result;
	}

	u32 Thread::PerformanceCoreCount()
	{
		u32 count = 0;
		for (const CoreInfo& c : EnumerateCores())
			if (c.isPerformance)
				++count;
		return count;
	}

	u32 Thread::EfficiencyCoreCount()
	{
		u32 count = 0;
		for (const CoreInfo& c : EnumerateCores())
			if (!c.isPerformance)
				++count;
		return count;
	}
}