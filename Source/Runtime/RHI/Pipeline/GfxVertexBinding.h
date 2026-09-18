#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/RHI/Texture/GfxTextureFormat.h>
#include <Runtime/RHI/Pipeline/GfxVertexInputRate.h>

#include <string_view>

namespace Horizon::RHI
{
	struct GfxVertexAttribute
	{
		std::string_view semantic;
		u32 semanticIndex = 0;
		GfxTextureFormat format = GfxTextureFormat::Undefined;
		u32 binding = 0;
		u32 offset = 0;
	};

	struct GfxVertexBinding
	{
		u32 binding = 0;
		u32 stride = 0;
		GfxVertexInputRate inputRate = GfxVertexInputRate::PerVertex;
	};
}