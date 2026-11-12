#pragma once

#include <Runtime/Math/Vec4f.h>

namespace Horizon::Asset
{
	struct MaterialData
	{
		Math::Vec4f baseColorFactor;
		Math::Vec4f emissiveFactor;
		f32 metallic;
		f32 roughness;
		f32 normalScale;
		f32 alphaCutoff;
		u32 baseColorIndex;
		u32 normalIndex;
		u32 emissiveIndex;
		u32 occlusionIndex;
		u32 roughnessIndex;
		u32 metallicIndex;
		u32 occlusionChannel;
		u32 roughnessChannel;
		u32 metallicChannel;
		u32 reserved;
	};
}