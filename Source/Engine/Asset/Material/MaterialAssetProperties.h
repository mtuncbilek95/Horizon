#pragma once

#include <Engine/Asset/Material/MaterialParamType.h>
#include <Runtime/Containers/Guid.h>
#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::Engine
{
	struct MaterialParamValue
	{
		u64 nameHash = 0;
		MaterialParamType type = MaterialParamType::Float;
		u8 value[16] = {};
		Guid textureId;
	};

	struct MaterialAssetProperties
	{
		Guid shaderId;
		u32 valueCount = 0;
	};
}