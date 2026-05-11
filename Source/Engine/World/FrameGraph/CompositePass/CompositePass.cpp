#include "CompositePass.h"

#include <Runtime/Graphics/RHI/Command/GfxCommandBuffer.h>
#include <Runtime/Graphics/RHI/Image/GfxImage.h>
#include <Runtime/Graphics/RHI/Image/GfxImageView.h>

#include <Engine/World/FrameGraph/GraphBuilder.h>
#include <Engine/World/FrameGraph/PassResources.h>

#include <Engine/Window/WindowSystem.h>

namespace Horizon
{
	CompositePass::CompositePass()
	{
		InputSingleton::Get().GetDispatcher()->OnKeyPressed([&](const InputMessage& msg)
			{
				if (msg.key == KeyCode::Space && msg.keyAction == InputAction::Press)
					m_switched = !m_switched;
			});
	}

	void CompositePass::Setup(GraphBuilder& builder)
	{
		m_colorHandle = builder.ReadFrom("RedPass", "color");
		m_switchHandle = builder.ReadFrom("BluePass", "color");
		m_backbufferHandle = builder.WriteToComposite();
	}

	void CompositePass::Execute(GfxCommandBuffer* cmd, const PassResources& resources)
	{
		GfxImageView* color = nullptr;
		if (m_switched)
			color = resources.Get(m_colorHandle);
		else
			color = resources.Get(m_switchHandle);

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
	}
}