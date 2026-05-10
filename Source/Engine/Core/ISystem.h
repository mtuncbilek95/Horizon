#pragma once

#include <Engine/Core/EngineReport.h>
#include <string_view>
#include <typeindex>

namespace Horizon
{
	class ISystem
	{
	public:
		virtual ~ISystem() = default;

		virtual EngineReport OnInitialize() = 0;
		virtual void OnSync() = 0;
		virtual void OnFinalize() = 0;

		virtual std::string GetObjectType() const = 0;
		virtual std::type_index GetObjectIndex() const = 0;

	protected:
		template<typename U>
		static std::string TypeName()
		{
			std::string name = __FUNCSIG__;
			auto start = name.rfind("TypeName<") + 9;

			auto end = name.rfind(">");
			name = name.substr(start, end - start);

			if (name.contains("class"))
				name = name.substr(6);

			if (name.contains("struct"))
				name = name.substr(7);

			return name;
		}
	};
}