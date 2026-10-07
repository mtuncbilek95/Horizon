#include "SettingsContext.h"

#include <Engine/Core/ModuleGraph.h>
#include <Engine/Plugin/PluginService.h>

namespace Horizon::Engine
{
	ModuleReport SettingsContext::OnInitialize()
	{
		return ModuleReport();
	}

	void SettingsContext::OnFinalize()
	{
	}

	void SettingsContext::DeclareDependencies(ModuleGraph& graph)
	{
		graph.Requires<PluginService>();
	}
}