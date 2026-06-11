#pragma once

#include <Engine/Engine/IModule.h>
#include <Engine/ECS/WorldRegistry.h>

namespace Horizon
{
	class EntityComponentModule : public IModule
	{
	public:
		void OnAttach(Engine& engine) final;
		void OnSync() final;
		void OnDetach() final;

	private:
		WorldRegistry m_world;
	};
}