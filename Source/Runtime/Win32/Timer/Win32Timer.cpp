#include <Runtime/PAL/Timer/Timer.h>

#include <Windows.h>

namespace Horizon::PAL
{
	namespace 
	{
		i64 QueryFreq() 
		{
			LARGE_INTEGER li;
			QueryPerformanceFrequency(&li);
			return li.QuadPart;
		}

		i64 QueryCounter() 
		{
			LARGE_INTEGER li;
			QueryPerformanceCounter(&li);
			return li.QuadPart;
		}
	}

	void Timer::Start()
	{
		m_frequency = QueryFreq();
		m_start = QueryCounter();
	}

	void Timer::Reset()
	{
		m_start = QueryCounter();
	}

	f64 Timer::GetElapsedTimeInSec() const
	{
		i64 now = QueryCounter();
		return static_cast<f64>(now - m_start) / static_cast<f64>(m_frequency);
	}

	f64 Timer::GetElapsedTimeInMs() const
	{
		return GetElapsedTimeInSec() * 1000.0;
	}
}