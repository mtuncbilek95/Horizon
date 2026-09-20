#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::Editor
{
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
		InvalidMeshData,
		SerializationFailed,
		InternalError
	};
}