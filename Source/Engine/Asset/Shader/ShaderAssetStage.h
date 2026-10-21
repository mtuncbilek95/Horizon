#pragma once

#include <Runtime/RHI/Shader/GfxShaderStage.h>

namespace Horizon::Engine
{
	struct ShaderAssetStage
	{
		RHI::GfxShaderStage stage;
		u64 offset;
		u64 byteSize;
	};
}