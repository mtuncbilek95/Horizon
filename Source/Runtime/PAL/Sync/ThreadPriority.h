#pragma once

namespace Horizon
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