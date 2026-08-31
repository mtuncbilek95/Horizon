#pragma once

#include <Runtime/Containers/Guid.h>
#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::Engine
{
	static constexpr u32 ContentFileMagic = 0x4C494648;
	static constexpr u32 ContentFileVersion = 1;
	static constexpr u32 ContentSectionAlignment = 256;

	static constexpr u32 ContentSectionFlagBlittable = 1u << 0;
	static constexpr u32 ContentSectionFlagResident = 1u << 1;
	static constexpr u32 ContentSectionFlagCompressed = 1u << 2;

	struct ContentSectionDesc
	{
		u32 sectionId = kInvalid32;
		u32 flags = 0;
		u64 dataOffset = 0;
		u64 storedSize = 0;
		u64 decodedSize = 0;
	};

	struct ContentFileHeader
	{
		u32 magic = ContentFileMagic;
		u32 containerVersion = ContentFileVersion;
		u32 payloadVersion = 0;
		u32 sectionCount = 0;
		Guid id;
		u64 typeNameOffset = 0;
		u64 dependencyOffset = 0;
		u32 dependencyCount = 0;
		u32 padding = 0;
		u64 sectionTableOffset = 0;
	};
}