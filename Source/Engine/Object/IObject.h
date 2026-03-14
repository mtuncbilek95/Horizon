#pragma once

#include <string_view>
#include <typeindex>

namespace Horizon
{
	class IObject
	{
	public:
		virtual ~IObject() = default;

		virtual std::string_view GetObjectType() const = 0;
		virtual std::type_index GetObjectIndex() const = 0;

		template<typename U>
		U& As()
		{
			if (GetObjectIndex() != typeid(U))
				throw std::bad_cast();

			return static_cast<U&>(*this);
		}

	protected:
		template<typename U>
		static std::string_view TypeName()
		{
			std::string_view name = __FUNCSIG__;

			auto start = name.rfind("TypeName<") + 9;
			auto end = name.rfind(">(");
			name = name.substr(start, end - start);

			if (name.starts_with("class "))
				name.remove_prefix(6);
			if (name.starts_with("struct "))
				name.remove_prefix(7);

			auto ns = name.rfind("::");
			if (ns != std::string_view::npos)
				name = name.substr(ns + 2);

			return name;
		}
	};
}