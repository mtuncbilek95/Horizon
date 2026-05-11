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
		// Get RenderableObject
		/*
		std::vector<RenderableObject> renderables;
		for(entity related loop)
		{
			// Get mesh, textures, etc and put it in RenderableObject.
		}
		m_graph->SetRenderables(renderables);
		*/

		// Get CameraComponent
		/*
		ViewObject viewObj;
		//Fill it and send
		m_graph->SetViewObject(viewObj);
		*/

		auto& presentSystem = GetECS()->RequestSystem<PresentationSystem>();

		CompositePresentObject presentObj = presentSystem.AcquireNextImage();
		m_graph->Import(presentObj);
		m_graph->Compile();
		m_graph->Execute(m_cmdBuffers[presentObj.frameIndex].get());

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