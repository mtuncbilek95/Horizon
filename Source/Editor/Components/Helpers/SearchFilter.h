#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <cctype>
#include <string>

namespace Horizon::Editor
{
	struct SearchFilter
	{
		static b8 Matches(const std::string& name, const std::string& search);
	};
}