#include "ProjectContext.h"

#include <Engine/Plugin/PluginSystem.h>
#include <Engine/Window/WindowService.h>
#include <Runtime/Containers/StringOps.h>
#include <Runtime/PAL/File/Directory.h>

namespace Horizon::Editor
{
	ProjectContext::ProjectContext(const std::string& projectPath, const std::string& engineResourcePath) : m_projectPath(projectPath),
		m_engineResourcePath(engineResourcePath)
	{
	}

	Engine::ModuleReport ProjectContext::OnInitialize()
	{
		m_projectPath = StringOps::NormalizePath(m_projectPath);

		usize index = m_projectPath.rfind('/');
		if (index != std::string::npos)
		{
			m_projectName = m_projectPath.substr(index + 1);
			m_projectPath = m_projectPath.substr(0, index);
		}

		m_assetPath = m_projectPath + "/Assets";
		m_cookPath = m_projectPath + "/Cooked";
		m_pluginPath = m_projectPath + "/Plugins";

		if (!PAL::Directory::Exists(m_assetPath) && !PAL::Directory::Create(m_assetPath))
			return Engine::ModuleReport("Asset root cannot be created");

		if (!PAL::Directory::Exists(m_cookPath) && !PAL::Directory::Create(m_cookPath))
			return Engine::ModuleReport("Cook root cannot be created");

		if (!PAL::Directory::Exists(m_pluginPath) && !PAL::Directory::Create(m_pluginPath))
			return Engine::ModuleReport("Plugin root cannot be created");

		auto* pPlugins = GetEngine()->GetPluginSystem();

		pPlugins->SetRoots("", m_pluginPath);
		pPlugins->SetEnabledPlugins(m_enabledPlugins);
		pPlugins->DiscoverPlugins();
		pPlugins->LoadPlugins();

		return Engine::ModuleReport();
	}

	void ProjectContext::OnFinalize()
	{
	}

	void ProjectContext::DeclareDependencies(Engine::ModuleGraph& graph)
	{
		graph.Precedes<Engine::WindowService>();
	}

	void ProjectContext::SetPluginEnabled(const std::string& name, b8 enabled)
	{
		const b8 current = m_enabledPlugins.Contains(name);

		if (enabled == current)
			return;

		if (enabled)
			m_enabledPlugins.PushBack(name);
		else
			m_enabledPlugins.Remove(name);
	}
}