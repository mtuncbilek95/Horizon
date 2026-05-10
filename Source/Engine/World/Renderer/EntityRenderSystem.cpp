#include "EntityRenderSystem.h"

#include <Runtime/Graphics/RHI/Device/GfxDevice.h>
#include <Runtime/Graphics/RHI/Command/GfxCommandBuffer.h>
#include <Runtime/Graphics/RHI/Image/GfxImageView.h>
#include <Runtime/Graphics/RHI/Queue/GfxQueue.h>
#include <Runtime/Graphics/RHI/Sync/GfxSemaphore.h>

#include <Engine/Graphics/GfxContext.h>
#include <Engine/Presentation/PresentationSystem.h>
#include <Engine/World/EntityComponentSystem.h>
#include <Engine/World/FrameGraph/FrameGraph.h>
#include <Engine/World/FrameGraph/FrameGraphCache.h>

namespace Horizon
{
	EntityRenderSystem::EntityRenderSystem() : m_graph(nullptr), m_device(nullptr),
		m_gQueue(nullptr)
	{}

	EntityRenderSystem::~EntityRenderSystem()
	{}

	void EntityRenderSystem::OnTick()
	{
		auto& presentSystem = GetECS()->RequestSystem<PresentationSystem>();

		CompositePresentObject presentObj = presentSystem.AcquireNextImage();
		m_graph->ResolveCompositePass(presentObj);

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

	b8 EntityRenderSystem::OnInitialize()
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

		// Generate cache without telling graph anything about GfxDevice
		auto graphCache = std::make_unique<FrameGraphCache>(m_device);
		m_graph = std::make_unique<FrameGraph>();
		m_graph->SetGraphCache(std::move(graphCache));

		return true;
	}

	void EntityRenderSystem::OnSync()
	{}

	void EntityRenderSystem::OnFinalize()
	{}
}