#pragma once

#include <Application/Engine/System.h>

namespace Horizon
{
	class EntityComponentSystem final : public System<EntityComponentSystem>
	{
	public:
		EntityComponentSystem() = default;
		~EntityComponentSystem() = default;

	private:
		EngineReport OnInitialize() final;
		void OnSync() final;
		void OnFinalize() final;

	private:
	};
}