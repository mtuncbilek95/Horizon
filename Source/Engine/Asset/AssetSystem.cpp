#include "AssetSystem.h"

#include <Engine/Core/Engine.h>
#include <Engine/Job/JobSystem.h>

namespace Horizon
{
	SystemReport AssetSystem::OnAttach(Engine* pEngine)
	{
		System::OnAttach(pEngine);

		return SystemReport();
	}

	void AssetSystem::OnSync()
	{
	}

	void AssetSystem::OnDetach()
	{
	}

	void AssetSystem::GetInitializeOrder(OrderRules& rules) const
	{
		Requires<JobSystem>(rules.after);
	}
}