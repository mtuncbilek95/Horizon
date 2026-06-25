#pragma once

#include <Engine/Core/EngineReport.h>

#include <vector>
#include <typeindex>

namespace Horizon
{
	class Engine;

	enum class OrderTier : u8
	{
		First,
		Default,
		Last
	};

	struct OrderRules
	{
		OrderTier tier = OrderTier::Default;
		std::vector<std::type_index> after;
		std::vector<std::type_index> before;
	};

	class H_EXPORT Subsystem
	{
	public:
		Subsystem() = default;
		virtual ~Subsystem() = default;

		Subsystem(const Subsystem&) = delete;
		Subsystem& operator=(const Subsystem&) = delete;

		virtual EngineReport OnAttach(Engine* engine) { m_engine = engine; return EngineReport(); }
		virtual void OnSync() {}
		virtual void OnDetach() {}

		virtual OrderTier GetTier() const { return OrderTier::Default; }
		virtual void GetInitializeOrder(OrderRules& rules) const {}
		virtual void GetExecutionOrder(OrderRules& rules) const {}

		std::string_view GetName() const
		{
			std::string_view name = typeid(*this).name();

			if (size_t pos = name.rfind("::"); pos != std::string_view::npos)
				return name.substr(pos + 2);

			if (size_t pos = name.find(' '); pos != std::string_view::npos)
				return name.substr(pos + 1);

			return name;
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