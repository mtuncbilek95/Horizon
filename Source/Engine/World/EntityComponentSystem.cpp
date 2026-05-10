#include "EntityComponentSystem.h"

#include <Engine/World/Renderer/EntityRenderSystem.h>

namespace Horizon
{
	EngineReport EntityComponentSystem::OnInitialize()
	{
		m_renderer = std::make_shared<EntityRenderSystem>();
		m_renderer->SetECS(this);
		m_renderer->OnInitialize();
		return EngineReport();
	}

	void EntityComponentSystem::OnSync()
	{
		m_renderer->OnTick();
	}

	void EntityComponentSystem::OnFinalize()
	{
	}
}