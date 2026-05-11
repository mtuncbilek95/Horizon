#include "BluePass.h"

#include <Runtime/Graphics/RHI/Command/GfxCommandBuffer.h>
#include <Runtime/Graphics/RHI/Image/GfxImage.h>
#include <Runtime/Graphics/RHI/Image/GfxImageView.h>

#include <Engine/World/FrameGraph/GraphBuilder.h>
#include <Engine/World/FrameGraph/PassResources.h>

namespace Horizon
{
	void BluePass::Setup(GraphBuilder& builder)
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
	}

	void BluePass::Execute(GfxCommandBuffer* cmd, const PassResources& resources)
	{
		GfxImageView* color = resources.Get(m_colorHandle);
		GfxImageView* depth = resources.Get(m_depthHandle);

		// Color attachment barrier
		cmd->ImageBarrier(ImageBarrierDesc()
			.setImage(color->ImageOwner())
			.setOldLayout(ImageLayout::Undefined)
			.setNewLayout(ImageLayout::ColorAttachmentOptimal)
			.setAspect(ImageAspect::Color));

		// Depth attachment barrier
		cmd->ImageBarrier(ImageBarrierDesc()
			.setImage(depth->ImageOwner())
			.setOldLayout(ImageLayout::Undefined)
			.setNewLayout(ImageLayout::DepthStencilAttachmentOptimal)
			.setAspect(ImageAspect::Depth));

		// God damn rendering
		cmd->BeginRendering(RenderingInfo()
			.setRenderAreaExtent({ color->ImageOwner()->ImgSize().x, color->ImageOwner()->ImgSize().y })
			.addColorAttachment(RenderingAttachmentInfo()
				.setImageView(color)
				.setImageLayout(ImageLayout::ColorAttachmentOptimal)
				.setLoadOp(AttachmentLoad::Clear)
				.setStoreOp(AttachmentStore::Store)
				.setClearValue(ClearValue().setColor({ 0.f, 0.f, 1.f, 1.f })))
			/*.setDepthAttachment(RenderingAttachmentInfo()
				.setImageView(depth)
				.setImageLayout(ImageLayout::DepthStencilAttachmentOptimal)
				.setLoadOp(AttachmentLoad::Clear)
				.setStoreOp(AttachmentStore::Store)
				.setClearValue(ClearValue().setDepth(1.0f)))*/);

				// Put some shit here.

		cmd->EndRendering();
	}
}