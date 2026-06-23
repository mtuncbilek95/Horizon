#pragma once

namespace Horizon
{
	template <typename TLockable>
	class ScopedLock
	{
	public:
		explicit ScopedLock(TLockable& lockable) : m_lockable(lockable)
		{
			m_lockable.Lock();
		}

		~ScopedLock()
		{
			m_lockable.Unlock();
		}

		ScopedLock(const ScopedLock&) = delete;
		ScopedLock& operator=(const ScopedLock&) = delete;

		ScopedLock(ScopedLock&&) = delete;
		ScopedLock& operator=(ScopedLock&&) = delete;

	private:
		TLockable& m_lockable;
	};
}