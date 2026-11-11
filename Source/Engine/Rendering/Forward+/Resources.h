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

	HCLASS();
	struct SkyOutput final { HORIZON_DATA_REFLECT(SkyOutput); };

	HCLASS();
	struct ClusterList final { HORIZON_DATA_REFLECT(ClusterList); };

	HCLASS();
	struct PointLightList final { HORIZON_DATA_REFLECT(PointLightList); };

	HCLASS();
	struct ClusteredLightGrid final { HORIZON_DATA_REFLECT(ClusteredLightGrid); };

	HCLASS();
	struct ActiveCluster final { HORIZON_DATA_REFLECT(ActiveCluster); };
}