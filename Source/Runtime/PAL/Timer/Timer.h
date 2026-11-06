#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::PAL
{
	class Timer
	{
	public:
		void Start();
		void Reset();
		
		f64 GetElapsedTimeInSec() const;
		f64 GetElapsedTimeInMs() const;

	private:
		i64 m_frequency;
		i64 m_start;
	};
}