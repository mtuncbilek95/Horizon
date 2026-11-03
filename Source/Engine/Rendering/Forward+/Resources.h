#pragma once

#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Rendering::Resource
{
	HCLASS();
	struct SceneDepth final { HORIZON_DATA_REFLECT(SceneDepth); };

	HCLASS();
	struct LightGrid final { HORIZON_DATA_REFLECT(LightGrid); };
}