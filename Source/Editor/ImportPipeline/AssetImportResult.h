#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	HENUM();
	enum class AssetImportResult : u8
	{
		Success,
		FileNotFound,
		FileReadFailed,
		EmptyInput,
		InvalidFormat,
		CorruptedData,
		UnsupportedVersion,
		UnsupportedFeature,
		ExternalDataRequired,
		NoImportableData,
		InvalidData,
		SerializationFailed,
		InternalError
	};
}