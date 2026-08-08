#include "EntityComponentSystem.h"

#include <Engine/ECS/WorldRegistry.h>

namespace Horizon::Engine
{
	AppReport EntityComponentSystem::OnAttach(Application* engine)
	{
		return AppReport();
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