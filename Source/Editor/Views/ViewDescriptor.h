#pragma once

#include <Runtime/RTTR/Reflection.h>
#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <string>

namespace Horizon
{
	struct H_EXPORT ViewDescriptor
	{
		std::string displayName;
		b8 multiInstance;
		b8 openOnStart;

		Reflect::Type* pCoreType;
	};
}