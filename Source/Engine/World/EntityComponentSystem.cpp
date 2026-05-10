#include "EntityComponentSystem.h"

#include <Engine/World/Renderer/EntityRenderSystem.h>

namespace Horizon
{
	EngineReport EntityComponentSystem::OnInitialize()
	{
		AddSystem<EntityRenderSystem>();

		for (auto& system : m_initPendingSystems)
		{
			if (system->OnInitialize())
				m_systems[system->GetObjectIndex()] = std::move(system);
		}
		m_initPendingSystems.clear();

		return EngineReport();
	}

	void EntityComponentSystem::OnSync()
	{
		for (auto& [_, system] : m_systems)
			system->OnSync();

		for (auto& [_, system] : m_systems)
			system->OnTick();
	}

	void EntityComponentSystem::OnFinalize()
	{
		std::vector<IEntitySystem*> syss;
		syss.reserve(m_systems.size());
		for (auto& [key, val] : m_systems)
			syss.push_back(val.get());

		for (usize it = syss.size(); it > 0; it--)
			syss[it - 1]->OnFinalize();
		m_systems.clear();
	}
}