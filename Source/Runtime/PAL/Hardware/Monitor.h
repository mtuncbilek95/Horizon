#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/Containers/Vector/List.h>

namespace Horizon::PAL
{
	struct MonitorInfo
	{
		i32 posX, posY;
		u32 width, height;
		i32 workPosX, workPosY;
		u32 workWidth, workHeight;
		f32 dpiScale;
		b8 isPrimary;
	};

	struct Monitor
	{
		static List<MonitorInfo> EnumerateMonitors();
	};
}