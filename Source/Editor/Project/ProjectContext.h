#pragma once

#include <Engine/Core/Context.h>
#include <Engine/Core/Engine.h>

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

		const std::string& GetProjectName() const { return m_projectName; }
		const std::string& GetProjectPath() const { return m_projectPath; }
		const std::string& GetAssetPath() const { return m_assetPath; }
		const std::string& GetCookPath() const { return m_cookPath; }
		const std::string& GetEngineResourcePath() const { return m_engineResourcePath; }

	private:
		std::string m_projectName;
		std::string m_projectPath;
		std::string m_assetPath;
		std::string m_cookPath;
		std::string m_engineResourcePath;
	};
}