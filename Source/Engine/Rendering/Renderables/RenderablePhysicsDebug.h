#pragma once

#include <Runtime/Containers/Vector/List.h>
#include <Runtime/Math/Vec3f.h>

namespace Horizon::Engine
{
	struct DebugLineVertex
	{
		Math::Vec3f position;
		u32 color;
	};

	struct FramePhysicsDebug
	{
		List<DebugLineVertex> lines;
	};
}