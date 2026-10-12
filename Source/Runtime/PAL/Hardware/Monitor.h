#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/Containers/List.h>

namespace Horizon::PAL
{
	struct RUNTIME_API MonitorInfo
	{
		i32 posX, posY;
		u32 width, height;
		i32 workPosX, workPosY;
		u32 workWidth, workHeight;
		f32 dpiScale;
		b8 isPrimary;
	};

	struct RUNTIME_API Monitor
	{
		static List<MonitorInfo> EnumerateMonitors();
	};
}