#pragma once

#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Rendering::Resource
{
	HCLASS();
	struct SceneDepth final { HORIZON_DATA_REFLECT(SceneDepth); };

	HCLASS();
	struct SceneColor final { HORIZON_DATA_REFLECT(SceneColor); };

	HCLASS();
	struct LightGrid final { HORIZON_DATA_REFLECT(LightGrid); };
}