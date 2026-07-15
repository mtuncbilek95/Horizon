#include "AssetSystem.h"

#include <Engine/Core/Engine.h>

namespace Horizon
{
	EngineReport AssetSystem::OnAttach(Engine* pEngine)
	{
		System::OnAttach(pEngine);

		return EngineReport();
	}

	void AssetSystem::OnSync()
	{
	}

	void AssetSystem::OnDetach()
	{
	}

	void AssetSystem::GetInitializeOrder(OrderRules& rules) const
	{
	}

	void AssetSystem::GetExecutionOrder(OrderRules& rules) const
	{
	}
}