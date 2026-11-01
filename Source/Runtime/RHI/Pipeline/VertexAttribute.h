#pragma once

#include <Runtime/RHI/Texture/TextureFormat.h>

#include <string>

namespace Horizon::RHI
{
	struct VertexAttribute final
	{
		std::string semantic;
		u32 semanticIndex = 0;
		TextureFormat format = TextureFormat::Undefined;
		u32 binding = 0;
		u32 offset = 0;
	};
}
