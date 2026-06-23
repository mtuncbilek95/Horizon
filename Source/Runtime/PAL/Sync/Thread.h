#pragma once

#include <string_view>

namespace Horizon
{
	using ThreadEntry = void(*)(void* userData);
	using ThreadHandle = void*;
	using CustomUserData = void*;

	struct CoreInfo
	{
		u32 logicalIndex;
		b8 isPerformance;
	};

	class Thread
	{
	public:
		Thread() = default;
		Thread(ThreadEntry entry, CustomUserData userData, std::string_view name = {});
		~Thread();

		Thread(const Thread&) = delete;
		Thread& operator=(const Thread&) = delete;

		Thread(Thread&& other) noexcept;
		Thread& operator=(Thread&& other) noexcept;

		void Join();
		void Detach();
		b8 IsJoinable() const { return m_handle != nullptr; }

		u64 GetId() const { return m_id; }

		void SetAffinity(u64 coreMask);
		static std::vector<CoreInfo> EnumerateCores();

		static void SleepMs(u32 millisec);
		static void YieldCurrent();
		static u32 HardwareConcurrency();
		static u64 CurrentId();

		static u32 PerformanceCoreCount();
		static u32 EfficiencyCoreCount();

	private:
		ThreadHandle m_handle = nullptr;
		u64 m_id = 0;
	};
}