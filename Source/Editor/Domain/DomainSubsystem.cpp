#include "DomainSubsystem.h"

namespace Horizon
{
	EngineReport DomainSubsystem::OnAttach(Engine* pEngine)
	{
		return EngineReport();
	}

	void DomainSubsystem::OnSync()
	{
	}

	void DomainSubsystem::OnDetach()
	{
	}

	void DomainSubsystem::GetInitializeOrder(OrderRules& rules) const
	{
	}

	void DomainSubsystem::GetExecutionOrder(OrderRules& rules) const
	{
	}
}