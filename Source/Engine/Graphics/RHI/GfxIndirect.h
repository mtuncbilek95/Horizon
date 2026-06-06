#pragma once

#include <Engine/Graphics/RHI/GfxDevice.h>

#include <string_view>

namespace Horizon
{
	struct GfxCommandSignatureDesc
	{
		GfxIndirectType type = GfxIndirectType::DrawIndexed;
		u32 constantCount = 0;

		std::string_view debugName;
	};
}