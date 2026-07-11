#include "CommandSystem.h"

#include <Engine/Core/Engine.h>
#include <Engine/Job/JobSystem.h>

namespace Horizon
{
	CommandSystem::CommandSystem(i32 argC, c8** argV)
	{
		m_executionPath = argV[0];

		if (argV[1])
			m_projectPath = argV[1];

		// for(i32 i = 2; i < argC; i++)
		
		// Dummy Folder for testing purpose. If you see this and say "Oh! That's absolute path!",
		// you should go fuck yourself harder.
		m_projectPath = "D:/Projects/Horizon/ExampleProject";
	}

	SystemReport CommandSystem::OnAttach(Engine* pEngine)
	{
		System::OnAttach(pEngine);

		Terminal::Debug("CommandSystem", "Execution Path - {}", m_executionPath.string());
		Terminal::Debug("CommandSystem", "Project Path - {}", m_projectPath.string());

		return SystemReport();
	}

	void CommandSystem::OnDetach()
	{
	}

	void CommandSystem::GetInitializeOrder(OrderRules& rules) const
	{
		Requires<JobSystem>(rules.after);
	}
}