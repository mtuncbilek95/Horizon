#include "SettingsContext.h"

#include <Engine/Core/ModuleDependency.h>

namespace Horizon::Engine
{
	ModuleReport SettingsContext::OnInitialize()
	{
		return ModuleReport();
	}

	void SettingsContext::OnFinalize()
	{
	}

	void SettingsContext::DeclareDependencies(ModuleDependency& graph)
	{
	}
}