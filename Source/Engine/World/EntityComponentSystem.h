#pragma once

#include <Engine/Core/System.h>
#include <Engine/World/Core/IEntitySystem.h>

#include <entt/entt.hpp>

namespace Horizon
{
	class EntityRenderSystem;

	class EntityComponentSystem : public System<EntityComponentSystem>
	{
	public:
		template<typename T, typename... Args, typename = std::enable_if_t<std::derived_from<T, IEntitySystem>>>
		void AddSystem(Args&&... args)
		{
			auto system = std::make_unique<T>(std::forward<Args>(args)...);
			system->SetECS(this);
			m_initPendingSystems.push_back(std::move(system));
		}

		template<typename T, typename = std::enable_if_t<std::derived_from<T, IEntitySystem>>>
		b8 RemoveSystem()
		{
			auto typeIndex = std::type_index(typeid(T));
			if (!m_systems.contains(typeIndex))
				return false;

			m_finalizePendingSystems.push_back(std::move(m_systems[typeIndex]));
			m_systems.erase(typeIndex);

			return true;
		}

		template<typename T, typename = std::enable_if_t<std::derived_from<T, IEntitySystem>>>
		T& GetSystem()
		{
			auto typeIndex = std::type_index(typeid(T));
			if (!m_systems.contains(typeIndex))
				throw std::runtime_error("System not found");

			return *static_cast<T*>(m_systems[typeIndex].get());
		}

	private:
		EngineReport OnInitialize() final;
		void OnSync() final;
		void OnFinalize() final;

	private:
		std::vector<std::unique_ptr<IEntitySystem>> m_initPendingSystems;
		std::vector<std::unique_ptr<IEntitySystem>> m_finalizePendingSystems;
		std::unordered_map<std::type_index, std::unique_ptr<IEntitySystem>> m_systems;

		entt::registry m_world;
	};
}