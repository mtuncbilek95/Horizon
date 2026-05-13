#include "DummySystem.h"

#include <Engine/World/EntityComponentSystem.h>
#include <Engine/World/Renderer/EntityRenderSystem.h>
#include <Engine/World/FrameGraph/FrameGraph.h>

#include <Engine/World/FrameGraph/BasicPass/BasicPass.h>
#include <Engine/World/FrameGraph/RedPass/RedPass.h>
#include <Engine/World/FrameGraph/BluePass/BluePass.h>
#include <Engine/World/FrameGraph/CompositePass/CompositePass.h>

#include <Engine/Graphics/ShaderContext.h>
#include <Engine/Graphics/PipelineContext.h>

namespace Horizon
{
	EngineReport DummySystem::OnInitialize()
	{
		auto& ecs = RequestSystem<EntityComponentSystem>();
		auto& ecsRenderer = ecs.GetSystem<EntityRenderSystem>();
		auto& graph = ecsRenderer.GetGraph();

		graph.RegisterPass<RedPass>("RedPass");
		graph.RegisterPass<BluePass>("BluePass");
		graph.RegisterPass<CompositePass>("CompositePass");

		return EngineReport();
	}

	void DummySystem::OnSync()
	{}

	void DummySystem::OnFinalize()
	{}
}