#include "PresentationService.h"

#include <Engine/Core/ModuleDependency.h>
#include <Engine/Graphics/GraphicsContext.h>
#include <Engine/Window/WindowService.h>
#include <Engine/World/WorldService.h>

namespace Horizon::Engine
{
	ModuleReport PresentationService::OnInitialize()
	{
		return ModuleReport();
	}

	void PresentationService::OnExecute(const EngineFrame& ctx)
	{
	}

	void PresentationService::OnFinalize()
	{
	}

	void PresentationService::DeclareDependencies(ModuleDependency& graph)
	{
		graph.InitAfter<WindowService, GraphicsContext>();
		graph.InitBefore<WorldService>();

		graph.RunAfter<WorldService>();
	}

	void PresentationService::OnLibraryRegistered(const ReflectionLibrary& library)
	{
	}

	void PresentationService::OnLibraryUnregistered(const ReflectionLibrary& library)
	{
	}
}