#include "CompositePass.h"

#include <Runtime/Graphics/RHI/Command/GfxCommandBuffer.h>
#include <Runtime/Graphics/RHI/Image/GfxImage.h>
#include <Runtime/Graphics/RHI/Image/GfxImageView.h>

#include <Engine/World/FrameGraph/GraphBuilder.h>
#include <Engine/World/FrameGraph/PassResources.h>

namespace Horizon
{
	void CompositePass::Setup(GraphBuilder& builder)
	{
		m_colorHandle = builder.ReadFrom("BasicPass", "color");
		m_backbufferHandle = builder.WriteToComposite();
	}

	void CompositePass::Execute(GfxCommandBuffer* cmd, const PassResources& resources)
	{
		GfxImageView* color = resources.Get(m_colorHandle);
		GfxImageView* back = resources.Get(m_backbufferHandle);

		cmd->ImageBarrier(ImageBarrierDesc()
			.setImage(color->ImageOwner())
			.setOldLayout(ImageLayout::ColorAttachmentOptimal)
			.setNewLayout(ImageLayout::TransferSrcOptimal)
			.setAspect(ImageAspect::Color));

		cmd->ImageBarrier(ImageBarrierDesc()
			.setImage(back->ImageOwner())
			.setOldLayout(ImageLayout::Undefined)
			.setNewLayout(ImageLayout::TransferDstOptimal)
			.setAspect(ImageAspect::Color));

		cmd->BlitImage(BlitImageDesc()
			.setSrcImage(color->ImageOwner())
			.setDstImage(back->ImageOwner())
			.setSrcSize(color->ImageOwner()->ImgSize())
			.setDstSize(back->ImageOwner()->ImgSize())
			.setSrcLayout(ImageLayout::TransferSrcOptimal)
			.setDstLayout(ImageLayout::TransferDstOptimal));

		cmd->ImageBarrier(ImageBarrierDesc()
			.setImage(back->ImageOwner())
			.setOldLayout(ImageLayout::TransferDstOptimal)
			.setNewLayout(ImageLayout::PresentSrcKHR)
			.setAspect(ImageAspect::Color));

		cmd->EndRecord();
	}
}