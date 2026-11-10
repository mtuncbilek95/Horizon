#pragma once

#include <Engine/Core/Context.h>
#include <Engine/Settings/EngineSettings.h>

namespace Horizon::Engine
{
	class SettingsContext : public Context
	{
	public:
		ModuleReport OnInitialize() final;
		void OnFinalize() final;
		void DeclareDependencies(ModuleDependency& graph) final;

		EngineSettings& GetSettings() { return m_settings; }
		const EngineSettings& GetSettings() const { return m_settings; }

	private:
		EngineSettings m_settings;
	};
}