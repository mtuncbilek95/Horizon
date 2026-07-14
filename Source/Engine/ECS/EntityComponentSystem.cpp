#include "EntityComponentSystem.h"

#include <Engine/ECS/WorldRegistry.h>

namespace Horizon
{
	EngineReport EntityComponentSystem::OnAttach(Engine* engine)
	{
		m_currentWorld = Allocator::Create<WorldRegistry>(CurrLoc());

		return EngineReport();
	}

	void EntityComponentSystem::OnDetach()
	{
		Allocator::Delete(m_currentWorld);
	}

	void EntityComponentSystem::GetInitializeOrder(OrderRules& rules) const
	{
	}

	void EntityComponentSystem::GetExecutionOrder(OrderRules& rules) const
	{
		// GameSubsystem will be added here
	}
}