#include "ImportSettingsRegistry.h"

#include <Editor/Attributes/ImportTypeAttribute.h>
#include <Engine/Module/ModuleContext.h>

namespace Horizon::Editor
{
	ImportSettingsRegistry::~ImportSettingsRegistry()
	{
		for (auto& set : m_registeredSettings)
			Memory::Allocator::Delete(set.pSettings);

		m_registeredSettings.Clear();
	}

	void ImportSettingsRegistry::BootstrapImportSettings(Engine::Application* pEngine)
	{
		auto* pModuleCtx = pEngine->GetModuleContext();
		List<Reflect::Type*> types = pModuleCtx->GetTypeByAttribute(Reflect::TypeOf<ImportTypeAttribute>());

		for (auto* type : types)
		{
			if (type->GetBaseId() != Reflect::TypeOf<ImportSettings>())
			{
				Terminal::Error("ImportSettingsRegistry", "It's not acceptable for {} to not inherit "
					"from ImportSettings", type->GetName());
				continue;
			}

			auto* pAttr = type->GetCustomAttribute<ImportTypeAttribute>();
			if (!pAttr)
				continue;

			ImportSettingsEntry entry = {};
			entry.pSettings = (ImportSettings*)type->CreateFromMemory();
			entry.handle = pAttr->GetType();
			entry.extensions = pAttr->GetExtensions();
			m_registeredSettings.PushBack(entry);
		}
	}

	List<ImportSettings*> ImportSettingsRegistry::GetSettingsByExtension(const std::string& ext) const
	{
		List<ImportSettings*> result;

		for (const auto& setting : m_registeredSettings)
		{
			if (setting.extensions.Contains(ext))
				result.PushBack(setting.pSettings);
		}

		return result;
	}

	ImportSettings* ImportSettingsRegistry::GetSettingsByAssetHandle(Reflect::TypeHandle handl) const
	{
		for (const auto& setting : m_registeredSettings)
		{
			if (setting.handle == handl)
				return setting.pSettings;
		}

		Terminal::Warn("ImportSettingRegistry", "There is no registered handl for what is being looked for!");
		return nullptr;
	}
}