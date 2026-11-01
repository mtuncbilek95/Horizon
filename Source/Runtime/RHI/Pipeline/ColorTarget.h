#pragma once

#include <Runtime/RHI/Pipeline/BlendState.h>
#include <Runtime/RHI/Texture/TextureFormat.h>

namespace Horizon::RHI
{
	struct ColorTarget final
	{
		TextureFormat format = TextureFormat::Undefined;
		BlendState blend;
	};
}
