#include "SettingsContext.h"

#include <Engine/Core/ModuleGraph.h>

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
	}
}