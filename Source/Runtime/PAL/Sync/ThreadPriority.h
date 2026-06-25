#pragma once

namespace Horizon::PAL
{
	enum class ThreadPriority : u64
	{
		Low,
		Normal,
		High,
		Highest,
		Realtime
	};
}