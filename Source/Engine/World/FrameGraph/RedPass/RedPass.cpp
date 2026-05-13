#include "RedPass.h"

#include <Runtime/Graphics/RHI/Command/GfxCommandBuffer.h>
#include <Runtime/Graphics/RHI/Image/GfxImage.h>
#include <Runtime/Graphics/RHI/Image/GfxImageView.h>

#include <Engine/Core/Engine.h>
#include <Engine/Graphics/PipelineContext.h>
#include <Engine/World/FrameGraph/FrameGraph.h>
#include <Engine/World/FrameGraph/GraphBuilder.h>
#include <Engine/World/FrameGraph/PassResources.h>

namespace Horizon
{
	void RedPass::Setup(GraphBuilder& builder)
	{
		m_colorHandle = builder.Create("color", RenderTargetDesc{
			.size = {1920, 1080},
			.usage = ImageUsage::ColorAttachment,
			.format = ImageFormat::R8G8B8A8_UNorm
			});

		m_depthHandle = builder.Create("depth", RenderTargetDesc{
			.size = {1920, 1080},
			.usage = ImageUsage::DepthStencil,
			.format = ImageFormat::D32_SFloat
			});

		m_colorHandle = builder.Write(m_colorHandle);
		m_depthHandle = builder.Write(m_depthHandle);

		m_testPipeline = GetGraph().GetEngine()->GetContext<PipelineContext>().GetOrCreate(PipelineInfo()
			.setCacheName("RedPass_BasicGraphics")
			.setShaderProgram("redPass::Graphics")
			.setTopology(PrimitiveTopology::TriangleList)
			.setRasterizer(RasterizerState()
				.setCullMode(CullMode::Back)
				.setPolygonMode(PolygonMode::Fill)
				.setFaceOrientation(FaceOrientation::CW)
				.setDepthBiasEnable(false))
			.setDepthStencil(DepthStencilState()
				.setDepthTestEnable(false)
				.setDepthWriteEnable(false)
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
				.addColorAttachmentFormat(ImageFormat::R8G8B8A8_UNorm))
			.addDynamicState(DynamicState::Viewport)
			.addDynamicState(DynamicState::Scissor));
	}

	void RedPass::Execute(GfxCommandBuffer* cmd, const PassResources& resources)
	{
		GfxImageView* color = resources.Get(m_colorHandle);
		GfxImageView* depth = resources.Get(m_depthHandle);

		// Color attachment barrier
		cmd->ImageBarrier(ImageBarrierDesc()
			.setImage(color->ImageOwner())
			.setOldLayout(ImageLayout::Undefined)
			.setNewLayout(ImageLayout::ColorAttachmentOptimal)
			.setAspect(ImageAspect::Color)
			.setOldStage(PipelineStageFlags::TopOfPipe)
			.setNewStage(PipelineStageFlags::ColorAttachment));

		cmd->BeginRendering(RenderingInfo()
			.setRenderAreaExtent({ color->ImageOwner()->ImgSize().x, color->ImageOwner()->ImgSize().y })
			.addColorAttachment(RenderingAttachmentInfo()
				.setImageView(color)
				.setImageLayout(ImageLayout::ColorAttachmentOptimal)
				.setLoadOp(AttachmentLoad::Clear)
				.setStoreOp(AttachmentStore::Store)
				.setClearValue(ClearValue().setColor({ 0.3f, 0.2f, 0.1f, 1.f })))
		);

		cmd->BindPipeline(m_testPipeline);
		cmd->BindViewport({ 0.f, 0.f }, { 1920.f, 1080.f }, { 0.f, 1.f });
		cmd->BindScissor({ 0, 0 }, { 1920, 1080 });

		cmd->DrawVertexed(3, 0, 0, 1);

		cmd->EndRendering();
	}
}