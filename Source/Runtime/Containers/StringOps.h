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
		static std::string NoExtension(const std::string& path);
		static std::string OnlyExtension(const std::string& path);
		static std::string ToRelative(const std::string& absolutePath, const std::string& trimPath, const std::string& rootOf);
		static std::string ToAbsolute(const std::string& relativePath, const std::string& missingPath, const std::string& rootOf);
		static std::string RootOf(const std::string& relativePath);

		static std::string_view GetNameString(std::string_view name);

		template<typename T>
		static std::string_view GetName(T* obj)
		{
			return ParseName(typeid(*obj).name());
		}

	private:
		static std::string_view ParseName(std::string_view name);
	};
}