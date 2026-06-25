#pragma once

namespace Horizon::PAL
{
	using MutexHandle = void*;

	class H_EXPORT Mutex
	{
	public:
		Mutex();
		~Mutex();

		Mutex(const Mutex&) = delete;
		Mutex& operator=(const Mutex&) = delete;

		Mutex(Mutex&& other) noexcept;
		Mutex& operator=(Mutex&& other) noexcept;

		void Lock() const;
		void Unlock() const;
		b8 TryLock() const;

	private:
		MutexHandle m_handle = nullptr;
	};
}