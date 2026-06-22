#pragma once

namespace Horizon
{
	using CritSecHandl = void*;

	class H_EXPORT CriticalSection
	{
	public:
		CriticalSection();
		~CriticalSection();

		b8 TryLock() const;
		void Lock() const;
		void Unlock() const;

	private:
		CritSecHandl m_handle;
	};
}