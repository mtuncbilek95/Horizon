#include "RenderExtraction.h"

#include <Engine/World/SystemDependency.h>
#include <Engine/World/Components/Location/WorldTransform.h>
#include <Engine/World/Components/Rendering/MeshSelection.h>

#include <Engine/World/Systems/Location/LocalToWorld.h>

namespace Horizon::World::System
{
	b8 RenderExtraction::OnInitialize()
	{
		return true;
	}

	void RenderExtraction::OnExecute(const Engine::EngineFrame& ctx, Engine::Scene& currentScene)
	{
	}

	void RenderExtraction::OnFinalize()
	{
	}

	void RenderExtraction::DeclareDependencies(Engine::SystemDependency& graph)
	{
		graph.Reads<Component::WorldTransform>();
		graph.Reads<Component::MeshSelection>();
		graph.RunsAfter<LocalToWorld>();
	}
}