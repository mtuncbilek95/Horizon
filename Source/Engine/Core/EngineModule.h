#pragma once

#include <Engine/Core/EngineReport.h>
#include <Engine/Core/OrderRules.h>

namespace Horizon
{
	class Engine;

	class EngineModule
	{
		friend class Engine;
	public:
		virtual ~EngineModule() = default;

		virtual EngineReport OnAttach(Engine*) { return {}; }
		virtual void OnDetach() {}

		virtual void GetInitializeOrder(OrderRules&) const {}

		std::string_view GetName() const
		{
			std::string_view name = typeid(*this).name();

			if (size_t pos = name.rfind("::"); pos != std::string_view::npos)
				return name.substr(pos + 2);

			if (size_t pos = name.find(' '); pos != std::string_view::npos)
				return name.substr(pos + 1);

			return name;
		}

		std::type_index GetTypeId() const
		{
			std::type_index index = typeid(*this);
			return index;
		}

	protected:
		template<typename... Ts>
		static void Requires(std::vector<std::type_index>& out)
		{
			(out.push_back(typeid(Ts)), ...);
		}

	protected:
		Engine* m_engine = nullptr;
	};
}