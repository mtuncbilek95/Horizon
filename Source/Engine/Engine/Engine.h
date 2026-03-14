#pragma once

#include <Engine/System/ISystem.h>

#include <memory>
#include <typeindex>
#include <unordered_map>

namespace Horizon
{
	class Engine
	{
	public:
		Engine() = default;
		~Engine() = default;

		/*
		* Adds new systems to the engine. The system is created and added to a pending initialization list, 
		* which will be processed in the next engine run loop.
		*/
		template<typename T, typename... Args, typename = std::enable_if_t<std::derived_from<T, ISystem>>>
		void AddSystem(Args&&... args)
		{
			// Create a new system and add it to the pending initialization list.
			auto system = std::make_unique<T>(std::forward<Args>(args)...);
			system->SetEngine(this);
			m_initPendingSystems.push_back(std::move(system));
		}

		/*
		* Remove System from the engine. The system is added to a pending finalization list, which will be 
		* processed in the next engine run loop.
		*/
		template<typename T, typename = std::enable_if_t<std::derived_from<T, ISystem>>>
		SystemReport RemoveSystem()
		{
			auto typeIndex = std::type_index(typeid(T));
			if (!m_systems.contains(typeIndex))
				return SystemReport(T::TypeName(), "System not found");

			// Add the system to the pending finalization list and remove it from the system map.
			m_finalizePendingSystems.push_back(std::move(m_systems[typeIndex]));
			m_systems.erase(typeIndex);

			return SystemReport();
		}

		/*
		* Retrieves a system from the engine. Throws an exception if the system is not found.
		*/
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