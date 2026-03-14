#pragma once

#include <vector>

namespace Horizon
{
	class GfxCommandBuffer;

	struct PresentRequest
	{
		GfxCommandBuffer* cmdBuffer;
		u32 imageIndex;

		PresentRequest& setCmdBuffer(GfxCommandBuffer* cmd) { cmdBuffer = cmd; return *this; }
		PresentRequest& setImageIndex(u32 index) { imageIndex = index; return *this; }
	};
}