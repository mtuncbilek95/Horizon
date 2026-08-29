#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <Runtime/RHI/Shader/GfxShaderStage.h>

namespace Horizon::RHI
{
	struct GfxShaderDesc
	{
		GfxShaderStage stage = GfxShaderStage::None;
		
		const void* pByteCode = nullptr;
		usize byteCodeSize = 0;
	};
}
