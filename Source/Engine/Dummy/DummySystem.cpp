#include "DummySystem.h"

#include <Engine/World/EntityComponentSystem.h>
#include <Engine/World/Renderer/EntityRenderSystem.h>
#include <Engine/World/FrameGraph/FrameGraph.h>

#include <Engine/World/FrameGraph/BasicPass/BasicPass.h>
#include <Engine/World/FrameGraph/RedPass/RedPass.h>
#include <Engine/World/FrameGraph/BluePass/BluePass.h>
#include <Engine/World/FrameGraph/CompositePass/CompositePass.h>

namespace Horizon
{
	EngineReport DummySystem::OnInitialize()
	{
		auto& ecs = RequestSystem<EntityComponentSystem>();
		auto& ecsRenderer = ecs.GetSystem<EntityRenderSystem>();
		auto& graph = ecsRenderer.GetGraph();

		graph.RegisterPass<BasicPass>("BasicPass");
		graph.RegisterPass<RedPass>("RedPass");
		graph.RegisterPass<BluePass>("BluePass");
		graph.RegisterPass<CompositePass>("CompositePass");

		// Add more stuff for ecs like add at least one mesh to see the behaviour.

		return EngineReport();
	}

	void DummySystem::OnSync()
	{}

	void DummySystem::OnFinalize()
	{}
}