#pragma once

#include <vector>
#include <typeindex>

namespace Horizon
{
	class Engine;

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