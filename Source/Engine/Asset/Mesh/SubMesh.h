#pragma once

namespace Horizon::Engine
{
	struct H_EXPORT SubMesh
	{
		u32 indexOffset = 0;
		u32 indexCount = 0;
		u32 materialSlot = 0;
	};
}