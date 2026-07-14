#include "CommandContext.h"

#include <Engine/Core/Engine.h>
#include <Engine/Job/JobContext.h>

namespace Horizon
{
	CommandContext::CommandContext(i32 argC, c8** argV)
	{
		m_executionPath = argV[0];

		if (argV[1])
			m_projectPath = argV[1];

		// for(i32 i = 2; i < argC; i++)
		
		// Dummy Folder for testing purpose. If you see this and say "Oh! That's absolute path!",
		// you should go fuck yourself harder.
		m_projectPath = "D:/Projects/Horizon/ExampleProject";
	}

	EngineReport CommandContext::OnAttach(Engine* pEngine)
	{
		Context::OnAttach(pEngine);

		Terminal::Debug("CommandContext", "Execution Path - {}", m_executionPath.string());
		Terminal::Debug("CommandContext", "Project Path - {}", m_projectPath.string());

		return EngineReport();
	}

	void CommandContext::OnDetach()
	{
	}

	void CommandContext::GetInitializeOrder(OrderRules& rules) const
	{
		Requires<JobContext>(rules.after);
	}
}