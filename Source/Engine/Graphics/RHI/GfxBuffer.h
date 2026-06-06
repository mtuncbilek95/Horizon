#pragma once

#include <Engine/Graphics/RHI/GfxDevice.h>

#include <string_view>

namespace Horizon
{
	struct GfxBufferDesc
	{
		usize size = 0;
		u32 stride = 0;
		GfxBufferUsage usage = GfxBufferUsage::None;
		GfxMemoryType memory = GfxMemoryType::GPU;
		std::string_view debugName = "";
	};
}