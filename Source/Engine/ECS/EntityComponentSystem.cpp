#include "EntityComponentSystem.h"

#include <Engine/ECS/WorldRegistry.h>

namespace Horizon
{
	EngineReport EntityComponentSystem::OnAttach(Engine* engine)
	{
		return EngineReport();
	}

	void EntityComponentSystem::OnDetach()
	{
	}

	void EntityComponentSystem::GetInitializeOrder(OrderRules& rules) const
	{
	}

	void EntityComponentSystem::GetExecutionOrder(OrderRules& rules) const
	{
		// GameSubsystem will be added here
	}
}