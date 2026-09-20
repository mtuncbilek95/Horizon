#pragma once

#include <Runtime/Containers/Guid.h>
#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::Engine
{
	struct AssetHeader
	{
		static constexpr u32 Magic = 0x54534148;
		static constexpr u32 Version = 1;

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