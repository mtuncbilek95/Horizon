#pragma once

#include <vector>
#include <typeindex>

namespace Horizon
{
	class Engine;

	enum class ExecutionTier : u8 
	{
		First,
		Default,
		Last
	};

	class H_EXPORT Subsystem
	{
	public:
		Subsystem() = default;
		virtual ~Subsystem() = default;

		Subsystem(const Subsystem&) = delete;
		Subsystem& operator=(const Subsystem&) = delete;

		virtual void OnAttach(Engine* engine) { m_engine = engine; }
		virtual void OnSync() {}
		virtual void OnDetach() {}

		virtual void GetExecuteAfter(std::vector<std::type_index>& out) const {}
		virtual void GetExecuteBefore(std::vector<std::type_index>& out) const {}
		virtual ExecutionTier GetExecutionTier() const { return ExecutionTier::Default; }

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