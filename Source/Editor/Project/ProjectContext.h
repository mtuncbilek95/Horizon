#pragma once

#include <Engine/Core/Context.h>

#include <filesystem>
#include <string>

namespace Horizon
{
	class ProjectContext : public Context 
	{
	public:
		ProjectContext(i32 argC, c8** argV);
		~ProjectContext() = default;

		EngineReport OnAttach(Engine* pEngine);
		void OnDetach();

		void GetInitializeOrder(OrderRules& rules) const final;

		const std::filesystem::path& GetExecutionPath() const { return m_executionPath; }
		const std::filesystem::path& GetProjectFilePath() const { return m_projectFilePath; }
		const std::filesystem::path& GetProjectFolderPath() const { return m_projectFolderPath; }
		const std::filesystem::path& GetDomainPath() const { return m_domainPath; }
		const std::filesystem::path& GetSettingsPath() const { return m_settingsPath; }
		const std::filesystem::path& GetCookedPath() const { return m_cookedPath; }
		const std::filesystem::path& GetEditorResourcePath() const { return m_editorResourcePath; }

		const std::string& GetProjectName() const { return m_projectName; }

	private:
		std::filesystem::path m_executionPath;
		std::filesystem::path m_projectFilePath;
		std::filesystem::path m_projectFolderPath;
		std::filesystem::path m_domainPath;
		std::filesystem::path m_settingsPath;
		std::filesystem::path m_cookedPath;
		std::filesystem::path m_editorResourcePath;

		std::string m_projectName;
	};
}