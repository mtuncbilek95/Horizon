#include "ProjectContext.h"

#include <Editor/Domain/DomainSystem.h>

namespace Horizon
{
	ProjectContext::ProjectContext(i32 argC, c8** argV)
	{
		if (argC < 2)
			Terminal::Warn(GetName(), "If you're getting this warning, you're either on developer mode, or engine running without a project");

		m_executionPath = std::string(argV[0]);
		if (!m_executionPath.empty())
			Terminal::Info(GetName(), "Application executed from {}", m_executionPath);

		if (argV[1])
		{
			m_projectFilePath = argV[1];
		}
		else
		{
#if defined(USE_DEV_MODE)
			if (m_projectFilePath.empty())
			{
				m_projectFilePath = std::string(DEVELOPMENT_PROJECT);
				Terminal::Info(GetName(), "Application is using development project. {}", m_projectFilePath);
			}
#endif
		}

		m_editorResourcePath = std::string(HORIZON_RESOURCE_DIR);
		Terminal::Info(GetName(), "Editor Resource Path - {}", m_editorResourcePath);

		m_projectFolderPath = m_projectFilePath.parent_path();
		Terminal::Info(GetName(), "Project Root Path - {}", m_projectFolderPath);

		m_projectName = m_projectFilePath.stem().string();
		Terminal::Info(GetName(), "Project Name - {}", m_projectName);
		
		m_domainPath = m_projectFolderPath / "Assets";
		if (!std::filesystem::exists(m_domainPath))
			std::filesystem::create_directory(m_domainPath);
		Terminal::Info(GetName(), "Project Assets Path - {}", m_domainPath);

		m_cookedPath = m_projectFolderPath / "Cooked";
		if (!std::filesystem::exists(m_cookedPath))
			std::filesystem::create_directory(m_cookedPath);
		Terminal::Info(GetName(), "Project Cooked File Path - {}", m_cookedPath);

		m_settingsPath = m_projectFolderPath / "Settings";
		if (!std::filesystem::exists(m_settingsPath))
			std::filesystem::create_directory(m_settingsPath);
		Terminal::Info(GetName(), "Project Settings Path - {}", m_settingsPath);
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
		Requires<DomainSystem>(rules.before);
	}
}