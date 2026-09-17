#pragma once

#include <Runtime/Containers/Guid.h>
#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::Engine
{
	struct AssetHeader
	{
		u32 magic;
		u32 version;
		Guid id;
		c8 typeName[MaxTypeBufferLength];
		u64 propertyOffset;
		u64 propertySize;
		u64 payloadOffset;
		u64 payloadSize;
	};
}