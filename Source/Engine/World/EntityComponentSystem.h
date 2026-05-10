#pragma once

#include <Engine/Core/System.h>

#include <entt/entt.hpp>

namespace Horizon
{
	class EntityRenderSystem;

	class EntityComponentSystem : public System<EntityComponentSystem>
	{
	public:
		EntityRenderSystem* Renderer() const { return m_renderer.get(); }

	private:
		EngineReport OnInitialize() final;
		void OnSync() final;
		void OnFinalize() final;

	private:
		std::shared_ptr<EntityRenderSystem> m_renderer;

		entt::registry m_world;
	};
}