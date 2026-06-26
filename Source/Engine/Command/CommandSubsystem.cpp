#include "CommandSubsystem.h"

#include <Engine/Core/Engine.h>
#include <Engine/Job/JobSubsystem.h>

namespace Horizon
{
	CommandSubsystem::CommandSubsystem(i32 argC, c8** argV)
	{
		m_executionPath = argV[0];

		if (argV[1])
			m_projectPath = argV[1];

		// for(i32 i = 2; i < argC; i++)
	}

	EngineReport CommandSubsystem::OnAttach(Engine* pEngine)
	{
		Subsystem::OnAttach(pEngine);

		Terminal::Debug("CommandSubsystem", "Execution Path - {}", m_executionPath.string());
		Terminal::Debug("CommandSubsystem", "Project Path - {}", m_projectPath.string());

		return EngineReport();
	}

	void CommandSubsystem::OnDetach()
	{
	}

	void CommandSubsystem::GetInitializeOrder(OrderRules& rules) const
	{
		Requires<JobSubsystem>(rules.after);
	}
}