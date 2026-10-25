#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <string>
#include <string_view>

namespace Horizon
{
	struct RUNTIME_API StringHelper final
	{
		static b8 IsCharSeparator(c8 value);
		static b8 CheckIfEquals(std::string_view left, std::string_view right);
		static b8 CheckIfEqualsCaseSensitive(std::string_view left, std::string_view right);

		static std::string NormalizePath(const std::string& path);
		static std::string FindParentPath(const std::string& path);

		static std::string ResolveWithNoExtension(const std::string path);
		static std::string FindExtension(const std::string path);

		static std::string_view GetNameString(std::string_view name);

		template<typename T>
		static std::string_view GetTypeName(T* pObject)
		{
			return ParseTypeName(typeid(*pObject).name());
		}
	private:
		static std::string_view ParseTypeName(std::string_view typeName);
	};
}