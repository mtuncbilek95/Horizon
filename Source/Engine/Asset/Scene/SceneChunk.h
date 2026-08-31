#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::Engine
{
	static constexpr u32 SceneFormatVersion = 1;
	static constexpr u32 SceneChunkSectionId = 0;
	static constexpr u32 SceneChunkFlagBlittable = 1u << 0;
	static constexpr u32 SceneChunkFlagNeedsFixup = 1u << 1;

	struct SceneChunkDesc
	{
		u32 typeIndex = kInvalid32;
		u32 count = 0;
		u32 stride = 0;
		u32 flags = 0;
		u64 entityOffset = 0;
		u64 dataOffset = 0;
		u64 dataSize = 0;
	};
}