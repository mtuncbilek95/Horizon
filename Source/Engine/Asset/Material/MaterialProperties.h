#pragma once

#include <Engine/Asset/Material/MaterialDomain.h>

namespace Horizon::Asset
{
	struct MaterialProperties
	{
		MaterialDomain domain = MaterialDomain::Opaque;
		b8 twoSided = false;
	};
}