#pragma once

#include <Engine/Core/AppReport.h>
#include <Engine/Core/OrderRules.h>

namespace Horizon::Engine
{
	class Application;

	class AppModule
	{
		friend class Application;
	public:
		virtual ~AppModule() = default;

		virtual AppReport OnAttach(Application* pEngine) { return {}; }
		virtual void OnDetach() {}

		virtual void GetInitializeOrder(OrderRules& rules) const {}

		std::string_view GetName() const
		{
			std::string_view name = typeid(*this).name();

			if (usize pos = name.rfind("::"); pos != std::string_view::npos)
				return name.substr(pos + 2);

			if (usize pos = name.find(' '); pos != std::string_view::npos)
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
		static void Requires(List<std::type_index>& out)
		{
			(out.PushBack(typeid(Ts)), ...);
		}

	protected:
		Application* m_engine = nullptr;
	};
}