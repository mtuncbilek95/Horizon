#include "AssetSubsystem.h"

#include <Engine/Core/Engine.h>
#include <Engine/Job/JobSubsystem.h>

namespace Horizon
{
	EngineReport AssetSubsystem::OnAttach(Engine* pEngine)
	{
		Subsystem::OnAttach(pEngine);

		return EngineReport();
	}

	void AssetSubsystem::OnSync()
	{
	}

	void AssetSubsystem::OnDetach()
	{
	}

	void AssetSubsystem::GetInitializeOrder(OrderRules& rules) const
	{
		Requires<JobSubsystem>(rules.after);
	}
}