#pragma once

#include <Editor/Domain/ImportSettings/ImportSettings.h>
#include <Engine/Core/Application.h>
#include <Runtime/Containers/List.h>

namespace Horizon::Editor
{
	class H_EXPORT ImportSettingsRegistry
	{
		struct ImportSettingsEntry
		{
			ImportSettings* pSettings = nullptr;
			Reflect::TypeHandle handle;
			List<std::string> extensions;
		};
	public:
		ImportSettingsRegistry() = default;
		~ImportSettingsRegistry();

		void BootstrapImportSettings(Engine::Application* pEngine);

		List<ImportSettings*> GetSettingsByExtension(const std::string& ext) const;
		ImportSettings* GetSettingsByAssetHandle(Reflect::TypeHandle handl) const;

	private:
		List<ImportSettingsEntry> m_registeredSettings;
	};
}