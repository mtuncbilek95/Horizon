#include "ProjectContext.h"

#include <Editor/Domain/DomainSystem.h>

namespace Horizon
{
	ProjectContext::ProjectContext(i32 argC, c8** argV)
	{
	}

	EngineReport ProjectContext::OnAttach(Engine* pEngine)
	{
		return EngineReport();
	}

	void ProjectContext::OnDetach()
	{

	}

	void ProjectContext::GetInitializeOrder(OrderRules& rules) const
	{
		Requires<DomainSystem>(rules.after);
	}
}