#pragma once

#include <Application/Engine/ISystem.h>

#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace Horizon
{
	class Engine
	{
	public:
		Engine() = default;
		~Engine() = default;

		template<typename T, typename... Args, typename = std::enable_if_t<std::derived_from<T, ISystem>>>
		void AddSystem(Args&&... args)
		{
			auto system = std::make_unique<T>(std::forward<Args>(args)...);
			system->SetEngine(this);
			m_initPendingSystems.push_back(std::move(system));
		}

		template<typename T, typename = std::enable_if_t<std::derived_from<T, ISystem>>>
		EngineReport RemoveSystem()
		{
			auto typeIndex = std::type_index(typeid(T));
			if (!m_systems.contains(typeIndex))
				return EngineReport(T::TypeName(), "System not found");

			m_finalizePendingSystems.push_back(std::move(m_systems[typeIndex]));
			m_systems.erase(typeIndex);

			return EngineReport();
		}

		template<typename T, typename = std::enable_if_t<std::derived_from<T, ISystem>>>
		T& GetSystem()
		{
			auto typeIndex = std::type_index(typeid(T));
			if (!m_systems.contains(typeIndex))
				throw std::runtime_error("System not found");

			return *static_cast<T*>(m_systems[typeIndex].get());
		}

		template<typename T, typename = std::enable_if_t<std::derived_from<T, ISystem>>>
		bool HasSystem()
		{
			auto typeIndex = std::type_index(typeid(T));
			return m_systems.contains(typeIndex);
		}

		void Run();
		void RequestExit(std::string_view reason = "System requested");

	private:
		std::vector<std::unique_ptr<ISystem>> m_initPendingSystems;
		std::vector<std::unique_ptr<ISystem>> m_finalizePendingSystems;
		std::unordered_map<std::type_index, std::unique_ptr<ISystem>> m_systems;

		bool m_exitRequested = false;
	};
}