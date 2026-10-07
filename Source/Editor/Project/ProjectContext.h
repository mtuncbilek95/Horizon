#pragma once

#include <Engine/Core/Context.h>
#include <Engine/Core/Engine.h>
#include <Runtime/Containers/List.h>

#include <string>

namespace Horizon::Editor
{
	class EDITOR_API ProjectContext final : public Engine::Context
	{
	public:
		ProjectContext(const std::string& projectPath, const std::string& engineResourcePath);
		~ProjectContext() = default;

		Engine::ModuleReport OnInitialize() final;
		void OnFinalize() final;
		void DeclareDependencies(Engine::ModuleGraph& graph) final;

		void OnLibraryRegistered(const Engine::ReflectionLibrary& library) final {}
		void OnLibraryUnregistered(const Engine::ReflectionLibrary& library) final {}

		const std::string& GetProjectName() const { return m_projectName; }
		const std::string& GetProjectPath() const { return m_projectPath; }
		const std::string& GetAssetPath() const { return m_assetPath; }
		const std::string& GetCookPath() const { return m_cookPath; }
		const std::string& GetPluginPath() const { return m_pluginPath; }
		const std::string& GetEngineResourcePath() const { return m_engineResourcePath; }

		const List<std::string>& GetEnabledPlugins() const { return m_enabledPlugins; }
		b8 IsPluginEnabled(const std::string& name) const { return m_enabledPlugins.Contains(name); }
		void SetPluginEnabled(const std::string& name, b8 enabled);

	private:
		std::string m_projectName;
		std::string m_projectPath;
		std::string m_assetPath;
		std::string m_cookPath;
		std::string m_pluginPath;
		std::string m_engineResourcePath;

		List<std::string> m_enabledPlugins;
	};
}