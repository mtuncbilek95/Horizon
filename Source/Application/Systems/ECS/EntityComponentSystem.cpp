#include "EntityComponentSystem.h"

namespace Horizon
{
	EngineReport EntityComponentSystem::OnInitialize()
	{
		return EngineReport();
	}

	void EntityComponentSystem::OnSync()
	{}

	void EntityComponentSystem::OnFinalize()
	{}
}