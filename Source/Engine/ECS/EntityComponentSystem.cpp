#include "EntityComponentSystem.h"

#include <Engine/ECS/WorldRegistry.h>

namespace Horizon
{
	SystemReport EntityComponentSystem::OnAttach(Engine* engine)
	{
		m_currentWorld = Allocator::Create<WorldRegistry>(CurrLoc());

		return SystemReport();
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