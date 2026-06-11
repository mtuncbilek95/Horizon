#include "EntityComponentModule.h"

#include <Engine/Engine/Engine.h>

namespace Horizon
{
	void EntityComponentModule::OnAttach(Engine& engine)
	{
		IModule::OnAttach(engine);
	}

	void EntityComponentModule::OnSync()
	{}

	void EntityComponentModule::OnDetach()
	{}
}