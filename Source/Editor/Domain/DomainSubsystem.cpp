#include "DomainSubsystem.h"

#include <Engine/Core/Engine.h>
#include <Engine/Asset/AssetSubsystem.h>

namespace Horizon
{
	EngineReport DomainSubsystem::OnAttach(Engine* pEngine)
	{
		Subsystem::OnAttach(pEngine);

		return EngineReport();
	}

	void DomainSubsystem::OnSync()
	{
	}

	void DomainSubsystem::OnDetach()
	{
	}

	void DomainSubsystem::GetExecutionOrder(OrderRules& rules) const
	{
		Requires<AssetSubsystem>(rules.after);
	}
}