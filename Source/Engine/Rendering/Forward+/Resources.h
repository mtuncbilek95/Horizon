#pragma once

#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Rendering::Resource
{
	HCLASS();
	struct HiZHistory final { HORIZON_DATA_REFLECT(HiZHistory); };

	HCLASS();
	struct HiZPyramid final { HORIZON_DATA_REFLECT(HiZPyramid); };

	HCLASS();
	struct VisibleInstances final { HORIZON_DATA_REFLECT(VisibleInstances); };

	HCLASS();
	struct DrawArgs final { HORIZON_DATA_REFLECT(DrawArgs); };

	HCLASS();
	struct PreDepthOutput final { HORIZON_DATA_REFLECT(PreDepthOutput); };

	HCLASS();
	struct ShadowAtlas final { HORIZON_DATA_REFLECT(ShadowAtlas); };

	HCLASS();
	struct ShadowOutput final { HORIZON_DATA_REFLECT(ShadowOutput); };

	HCLASS();
	struct LightBuffer final { HORIZON_DATA_REFLECT(LightBuffer); };

	HCLASS();
	struct LightGrid final { HORIZON_DATA_REFLECT(LightGrid); };

	HCLASS();
	struct LightIndexList final { HORIZON_DATA_REFLECT(LightIndexList); };

	HCLASS();
	struct HDRColor final { HORIZON_DATA_REFLECT(HDRColor); };

	HCLASS();
	struct PostProcessOutput final { HORIZON_DATA_REFLECT(PostProcessOutput); };

	HCLASS();
	struct SceneColor final { HORIZON_DATA_REFLECT(SceneColor); };
}