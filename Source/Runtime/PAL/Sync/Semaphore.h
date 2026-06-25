#pragma once

namespace Horizon::PAL
{
	using SemaphoreHandle = void*;

	class H_EXPORT Semaphore
	{
	public:
		Semaphore(u32 initialCount = 0, u32 maxCount = i32_max);
		~Semaphore();

		Semaphore(const Semaphore&) = delete;
		Semaphore& operator=(const Semaphore&) = delete;
		Semaphore(Semaphore&&) = delete;
		Semaphore& operator=(Semaphore&&) = delete;

		void Acquire() const;
		b8 TryAcquire(u32 timeoutMs) const;
		void Release(u32 count = 1) const;

	private:
		SemaphoreHandle m_handle = nullptr;
	};
}