#include "EntityRenderSystem.h"

#include <Runtime/Graphics/RHI/Device/GfxDevice.h>
#include <Runtime/Graphics/RHI/Command/GfxCommandBuffer.h>
#include <Runtime/Graphics/RHI/Image/GfxImageView.h>
#include <Runtime/Graphics/RHI/Queue/GfxQueue.h>
#include <Runtime/Graphics/RHI/Sync/GfxSemaphore.h>

#include <Engine/Graphics/GfxContext.h>
#include <Engine/Presentation/PresentationSystem.h>
#include <Engine/World/EntityComponentSystem.h>

namespace Horizon
{
	EntityRenderSystem::EntityRenderSystem() : m_graph(nullptr)
	{
	}

	EntityRenderSystem::~EntityRenderSystem()
	{
	}

	void EntityRenderSystem::OnInitialize()
	{
		auto& gfxCtx = GetECS()->RequestContext<GfxContext>();
		m_device = &gfxCtx.Device();
		m_gQueue = &gfxCtx.GraphicsQueue();

		m_pool = m_device->CreateCommandPool(GfxCommandPoolDesc()
			.setQueue(&gfxCtx.GraphicsQueue())
			.setFlags(CommandPoolFlags::ResetCommandBuffer));

		for (size_t i = 0; i < PresentationSystem::MaxFramesInFlight; i++)
			m_cmdBuffers.push_back(m_device->CreateCommandBuffer(GfxCommandBufferDesc()
				.setLevel(CommandLevel::Primary)
				.setPool(m_pool.get())));
	}

	void EntityRenderSystem::OnTick()
	{
		auto& presentSystem = GetECS()->RequestSystem<PresentationSystem>();

		CompositePresentObject presentObj = presentSystem.AcquireNextImage();

		// Start recording and change image layout
		m_cmdBuffers[presentObj.frameIndex]->BeginRecord();
		m_cmdBuffers[presentObj.frameIndex]->ImageBarrier(ImageBarrierDesc()
			.setImage(presentObj.imageView->ImageOwner())
			.setOldLayout(ImageLayout::Undefined)
			.setNewLayout(ImageLayout::ColorAttachmentOptimal)
			.setAspect(ImageAspect::Color));


		// change image layout to present and end recording then send cmdBuffer to submission
		m_cmdBuffers[presentObj.frameIndex]->ImageBarrier(ImageBarrierDesc()
			.setImage(presentObj.imageView->ImageOwner())
			.setOldLayout(ImageLayout::ColorAttachmentOptimal)
			.setNewLayout(ImageLayout::PresentSrcKHR)
			.setAspect(ImageAspect::Color));
		m_cmdBuffers[presentObj.frameIndex]->EndRecord();
		presentSystem.SubmitPresent(CompositeSubmitObject()
			.addCmdBuffer(m_cmdBuffers[presentObj.frameIndex].get())
			.setFrameIndex(presentObj.frameIndex)
			.setImageIndex(presentObj.imageIndex));
	}
}