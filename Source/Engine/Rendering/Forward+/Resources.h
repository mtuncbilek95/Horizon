#pragma once

#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Rendering::Resource
{
	HCLASS();
	struct PreDepthOutput final { HORIZON_DATA_REFLECT(PreDepthOutput); };

	HCLASS();
	struct SceneColor final { HORIZON_DATA_REFLECT(SceneColor); };

	HCLASS();
	struct SceneColorMS final { HORIZON_DATA_REFLECT(SceneColorMS); };
}