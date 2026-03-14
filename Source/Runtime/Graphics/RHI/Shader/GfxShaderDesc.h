#pragma once

#include <Runtime/Graphics/RHI/Util/ShaderStage.h>

namespace Horizon
{
	struct GfxShaderDesc
	{
		ShaderStage stage;
		std::span<u32> byteCode;

		GfxShaderDesc& setStage(ShaderStage s) { stage = s; return *this; }
		GfxShaderDesc& setByteCode(std::span<u32> code) { byteCode = code; return *this; }
	};
}
