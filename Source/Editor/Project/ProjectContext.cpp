#include "ProjectContext.h"

#include <Engine/Window/WindowService.h>
#include <Runtime/Containers/StringHelper.h>
#include <Runtime/PAL/File/Directory.h>

namespace Horizon::Editor
{
	ProjectContext::ProjectContext(const std::string& projectPath, const std::string& engineResourcePath) : m_projectPath(projectPath),
		m_engineResourcePath(engineResourcePath)
	{
	}

	Engine::ModuleReport ProjectContext::OnInitialize()
	{
		m_projectPath = StringHelper::NormalizePath(m_projectPath);

		usize index = m_projectPath.rfind('/');
		if (index != std::string::npos)
		{
			m_projectName = m_projectPath.substr(index + 1);
			m_projectPath = m_projectPath.substr(0, index);
		}

		m_assetPath = m_projectPath + "/Assets";
		m_cookPath = m_projectPath + "/Cooked";
		m_settingsPath = m_projectPath + "/Settings";

		if (!PAL::Directory::Exists(m_assetPath) && !PAL::Directory::Create(m_assetPath))
			return Engine::ModuleReport("Asset root cannot be created");

		if (!PAL::Directory::Exists(m_cookPath) && !PAL::Directory::Create(m_cookPath))
			return Engine::ModuleReport("Cook root cannot be created");

		if (!PAL::Directory::Exists(m_settingsPath) && !PAL::Directory::Create(m_settingsPath))
			return Engine::ModuleReport("Settings root cannot be created");

		return Engine::ModuleReport();
	}

	void ProjectContext::OnFinalize()
	{
	}

	void ProjectContext::DeclareDependencies(Engine::ModuleDependency& graph)
	{
		graph.InitBefore<Engine::WindowService>();
	}
}