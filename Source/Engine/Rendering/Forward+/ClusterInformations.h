#pragma once

#include <Runtime/Math/Mat4f.h>
#include <Runtime/Math/Vec4f.h>

namespace Horizon::Rendering::Pass
{
	constexpr u32 kMaxLightsPerCluster = 256;
	constexpr u32 kClusterTileSize = 64;
	constexpr u32 kClusterCountZ = 24;

	struct ClusterAABB
	{
		Math::Vec4f minViewSpace;
		Math::Vec4f maxViewSpace;
	};

	struct ClusterBuildConstant
	{
		Math::Mat4f inverseProj;
		u32 clusterIndex;
		u32 width;
		u32 height;
		u32 tileSize;
		u32 clusterCountX;
		u32 clusterCountY;
		u32 clusterCountZ;
		f32 nearPlane;
		f32 farPlane;
	};

	struct MarkClusterConstant
	{
		u32 depthIndex;
		u32 activeFlagsIndex;
		u32 width;
		u32 height;
		u32 tileSize;
		u32 clusterCountX;
		u32 clusterCountY;
		u32 clusterCountZ;
		f32 nearPlane;
		f32 farPlane;
	};

	struct CullClusterConstant
	{
		Math::Mat4f viewMatrix;
		u32 clusterIndex;
		u32 pointLightIndex;
		u32 lightGridIndex;
		u32 lightCount;
		u32 clusterCountX;
		u32 clusterCountY;
		u32 clusterCountZ;
		u32 maxLightsPerCluster;
		u32 activeFlagsIndex;
	};
}