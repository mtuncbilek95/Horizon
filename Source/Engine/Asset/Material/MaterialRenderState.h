#pragma once

#include <Engine/Asset/Material/MaterialBlendMode.h>

namespace Horizon::Engine
{
	struct MaterialRenderState
	{
		MaterialBlendMode blendMode = MaterialBlendMode::Opaque;
		b8 twoSided = false;
	};
}