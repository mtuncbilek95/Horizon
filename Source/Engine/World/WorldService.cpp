#include "WorldService.h"

#include <Engine/Core/Engine.h>
#include <Engine/Core/ModuleGraph.h>
#include <Engine/Asset/AssetService.h>
#include <Engine/Graphics/GraphicsContext.h>

namespace Horizon::Engine
{
	ModuleReport WorldService::OnInitialize()
	{
		return ModuleReport();
	}

	void WorldService::OnExecute()
	{
	}

	void WorldService::OnFinalize()
	{
	}

	void WorldService::DeclareDependencies(ModuleGraph& graph)
	{
		graph.Requires<GraphicsContext>();
		graph.Requires<AssetService>();
	}
}