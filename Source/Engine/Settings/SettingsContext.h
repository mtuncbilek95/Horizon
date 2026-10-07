#pragma once

#include <Engine/Core/Context.h>
#include <Engine/Settings/EngineSettings.h>

namespace Horizon::Engine
{
	class ENGINE_API SettingsContext : public Context 
	{
	public:
		ModuleReport OnInitialize() final;
		void OnFinalize() final;
		void DeclareDependencies(ModuleGraph& graph) final;

	private:
		EngineSettings m_settings;
	};
}