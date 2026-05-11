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

		graph.RegisterPass<BasicPass>("BasicPass");
		graph.RegisterPass<RedPass>("RedPass");
		graph.RegisterPass<BluePass>("BluePass");
		graph.RegisterPass<CompositePass>("CompositePass");

		auto& shaderCtx = RequestContext<ShaderContext>();

		auto _1 = shaderCtx.GetProgram(PipelineType::Graphics, "triangle");
		auto _3 = shaderCtx.GetProgram(PipelineType::Compute, "triangle");
		auto _2 = shaderCtx.GetProgram(PipelineType::Raytracing, "test");

		auto& pipeCtx = RequestContext<PipelineContext>();

		auto* pipeline = pipeCtx.GetOrCreate(PipelineInfo()
			.setCacheName("Triangle_BasicGraphics")
			.setShaderProgram("triangle::Graphics")
			.setTopology(PrimitiveTopology::TriangleList)
			.setRasterizer(RasterizerState()
				.setCullMode(CullMode::Back)
				.setPolygonMode(PolygonMode::Fill)
				.setFaceOrientation(FaceOrientation::CCW)
				.setDepthBiasEnable(false))
			.setDepthStencil(DepthStencilState()
				.setDepthTestEnable(true)
				.setDepthWriteEnable(true)
				.setDepthOp(CompareOp::Less)
				.setDepthBoundEnable(false)
				.setMinDepth(0.0f)
				.setMaxDepth(1.0f))
			.setBlend(BlendState()
				.setLogicEnable(false)
				.addAttachment(BlendAttachment()
					.setBlendEnable(false)
					.setColorMask(ColorComponent::All)))
			.setDynamicRendering(DynamicRendering()
				.addColorAttachmentFormat(ImageFormat::R8G8B8A8_UNorm)
				.setDepthAttachmentFormat(ImageFormat::D32_SFloat))
			.addDynamicState(DynamicState::Viewport)
			.addDynamicState(DynamicState::Scissor));

		return EngineReport();
	}

	void DummySystem::OnSync()
	{}

	void DummySystem::OnFinalize()
	{}
}