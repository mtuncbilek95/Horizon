#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <string>
#include <string_view>

namespace Horizon
{
	struct H_EXPORT StringOps
	{
		static b8 IsSeparator(c8 value);
		static c8 ToLowerAscii(c8 value);
		static b8 StartsWithNoCase(const std::string& value, const std::string& prefix);
		static b8 EqualsNoCase(const std::string& left, const std::string& right);
		static std::string ParentPathOf(const std::string& path);
	};
}