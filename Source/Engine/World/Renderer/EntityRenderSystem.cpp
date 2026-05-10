#include "EntityRenderSystem.h"

#include <Runtime/Graphics/RHI/Device/GfxDevice.h>
#include <Runtime/Graphics/RHI/Command/GfxCommandBuffer.h>
#include <Runtime/Graphics/RHI/Image/GfxImage.h>
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
		RenderTargetHandle backbuffer = m_graph->Import(presentObj);

		RenderTargetHandle colorHandle;
		RenderTargetHandle depthHandle;

		m_graph->AddPass("BasicPass",
			[&](PassBuilder& builder)
			{
				colorHandle = builder.Create(RenderTargetDesc{
					.size = {1920, 1080},
					.usage = ImageUsage::ColorAttachment,
					.format = ImageFormat::R8G8B8A8_UNorm
					});

				depthHandle = builder.Create(RenderTargetDesc{
					.size = {1920, 1080},
					.usage = ImageUsage::DepthStencil,
					.format = ImageFormat::D32_SFloat
					});

				colorHandle = builder.Write(colorHandle);
				depthHandle = builder.Write(depthHandle);
			},
			[&](GfxCommandBuffer* cmd, const PassResources& resources)
			{
				GfxImageView* color = resources.Get(colorHandle);
				GfxImageView* depth = resources.Get(depthHandle);

				cmd->ImageBarrier(ImageBarrierDesc()
					.setImage(color->ImageOwner())
					.setOldLayout(ImageLayout::Undefined)
					.setNewLayout(ImageLayout::ColorAttachmentOptimal)
					.setAspect(ImageAspect::Color));

				cmd->BeginRendering(RenderingInfo()
					.setRenderAreaExtent({ color->ImageOwner()->ImgSize().x, color->ImageOwner()->ImgSize().y })
					.addColorAttachment(RenderingAttachmentInfo()
						.setImageView(color)
						.setImageLayout(ImageLayout::ColorAttachmentOptimal)
						.setLoadOp(AttachmentLoad::Clear)
						.setStoreOp(AttachmentStore::Store)
						.setClearValue(ClearValue().setColor({ 1.f, 1.f, 0.f, 1.f })))
					.setDepthAttachment(nullptr));

				cmd->EndRendering();
			}
		);

		// CompositePass
		m_graph->AddPass("CompositePass",
			[&](PassBuilder& builder)
			{
				builder.Read(colorHandle);
				builder.Write(backbuffer);
			},
			[&](GfxCommandBuffer* cmd, const PassResources& resources)
			{
				GfxImageView* color = resources.Get(colorHandle);
				GfxImageView* back = resources.Get(backbuffer);

				// color should go from attach to src
				cmd->ImageBarrier(ImageBarrierDesc()
					.setImage(color->ImageOwner())
					.setOldLayout(ImageLayout::ColorAttachmentOptimal)
					.setNewLayout(ImageLayout::TransferSrcOptimal)
					.setAspect(ImageAspect::Color));
				
				// back should go attach to dst
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

				// change image layout to present and end recording then send cmdBuffer to submission

			}
		);

		m_graph->Compile();

		m_cmdBuffers[presentObj.frameIndex]->BeginRecord();
		m_graph->Execute(m_cmdBuffers[presentObj.frameIndex].get());
		m_cmdBuffers[presentObj.frameIndex]->EndRecord();

		presentSystem.SubmitPresent(CompositeSubmitObject()
			.addCmdBuffer(m_cmdBuffers[presentObj.frameIndex].get())
			.setFrameIndex(presentObj.frameIndex)
			.setImageIndex(presentObj.imageIndex));

		m_graph->Reset();
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