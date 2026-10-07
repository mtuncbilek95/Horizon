#include "SettingsContext.h"

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