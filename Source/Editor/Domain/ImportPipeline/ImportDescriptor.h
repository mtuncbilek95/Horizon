#pragma once

#include <Runtime/RTTR/Reflection.h>
#include <string>

namespace Horizon
{
	struct H_EXPORT ImportDescriptor
	{
		std::string fileName;
		Reflect::Type* fileType;
	};
}