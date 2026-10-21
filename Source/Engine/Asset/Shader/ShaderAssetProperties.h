#pragma once

#include <Engine/Asset/Material/MaterialRenderState.h>
#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::Engine
{
	struct ShaderAssetProperties
	{
		MaterialRenderState renderState;
		u32 stageCount = 0;
	};
}