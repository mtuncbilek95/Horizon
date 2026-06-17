#pragma once

#include <Engine/Engine/IModule.h>
#include <Engine/ECS/Core/WorldRegistry.h>
#include <Engine/ECS/RenderableObject.h>

namespace Horizon
{
	class EntityComponentModule final : public IModule
	{
	public:
		void OnAttach(Engine& engine) final;
		void OnSync() final;
		void OnDetach() final;

		RenderView GetRenderView();

	private:
		WorldRegistry m_world;
	};
}