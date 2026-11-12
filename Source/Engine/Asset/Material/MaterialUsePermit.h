#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::Asset
{
	class Material;

	struct MaterialUsePermit
	{
		Material* pAsset = nullptr;
		u32 materialIndex = kInvalid32;

		b8 IsValid() const { return pAsset != nullptr; }
	};
}