#include "GraphicsModule.h"

#include <Engine/Engine/Engine.h>
#include <Engine/Window/WindowModule.h>
#include <Engine/Job/JobModule.h>

#include <cmath>

#include <imgui.h>

namespace Horizon
{
	void GraphicsModule::OnAttach(Engine& engine)
	{
		IModule::OnAttach(engine);

		auto* pWinModule = engine.TryGetModule<WindowModule>();

		m_mainDevice = Gfx::CreateGfxDevice(GfxDeviceDesc()
			.setDebug(true)
			.setGPUValidation(false));

		m_resourceHeap = Gfx::CreateGfxDescriptorHeap(m_mainDevice, GfxDescriptorHeapDesc()
			.setType(GfxDescriptorHeapType::Resource)
			.setCapacity(1 << 16)
			.setShaderVisible(true));

		m_colorHeap = Gfx::CreateGfxDescriptorHeap(m_mainDevice, GfxDescriptorHeapDesc()
			.setType(GfxDescriptorHeapType::Color)
			.setCapacity(1024)
			.setShaderVisible(false));

		m_depthHeap = Gfx::CreateGfxDescriptorHeap(m_mainDevice, GfxDescriptorHeapDesc()
			.setType(GfxDescriptorHeapType::Depth)
			.setCapacity(256)
			.setShaderVisible(false));

		m_graphicsQueue = Gfx::CreateGfxQueue(m_mainDevice, GfxQueueDesc()
			.setType(GfxQueueType::Graphics));

		m_computeQueue = Gfx::CreateGfxQueue(m_mainDevice, GfxQueueDesc()
			.setType(GfxQueueType::Compute));

		m_transferQueue = Gfx::CreateGfxQueue(m_mainDevice, GfxQueueDesc()
			.setType(GfxQueueType::Transfer));

		auto& currWindow = pWinModule->GetMainWindow();

		GfxSwapchainDesc scDesc = GfxSwapchainDesc()
			.setSync(true)
			.setWindowHandle(currWindow.GetOSHandle())
			.setImgSize(currWindow.GetSize().x, currWindow.GetSize().y)
			.setImgCount(3);
		m_swapchain = Gfx::CreateGfxSwapchain(m_mainDevice, m_graphicsQueue, scDesc);

		for (u32 i = 0; i < scDesc.imageCount; i++)
			Gfx::CreateTextureRTV(m_mainDevice, m_colorHeap, Gfx::RequestTexture(m_swapchain, i));

		m_globalLayout = Gfx::CreateGfxGlobalPipelineLayout(m_mainDevice);

		m_surfaceWidth = currWindow.GetSize().x;
		m_surfaceHeight = currWindow.GetSize().y;

		auto* pJobModule = engine.TryGetModule<JobModule>();
		m_threadCount = pJobModule ? pJobModule->WorkerCount() + 1 : 1;
		m_lanes.resize(m_threadCount * QueueTypeCount);

		Gfx::InitGfxImGui(m_mainDevice, m_graphicsQueue, m_resourceHeap,
			currWindow.GetAPIHandle(), GfxTextureFormat::RGBA8, MaxFramesInFlight);
	}

	GfxQueue* GraphicsModule::GetQueue(GfxQueueType type) const
	{
		return type == GfxQueueType::Compute ? m_computeQueue :
			type == GfxQueueType::Transfer ? m_transferQueue : m_graphicsQueue;
	}

	GfxTexture* GraphicsModule::ResolveTexture(GfxTextureHandle handl)
	{
		GfxTexture** ppTexture = m_texturePool.GetObject(handl);
		return ppTexture ? *ppTexture : nullptr;
	}

	GfxBuffer* GraphicsModule::ResolveBuffer(GfxBufferHandle handl)
	{
		GfxBuffer** ppBuffer = m_bufferPool.GetObject(handl);
		return ppBuffer ? *ppBuffer : nullptr;
	}

	GfxPipeline* GraphicsModule::ResolvePipeline(GfxPipelineHandle handl)
	{
		GfxPipeline** ppPipeline = m_pipelinePool.GetObject(handl);
		return ppPipeline ? *ppPipeline : nullptr;
	}

	u32 GraphicsModule::GetTextureShaderView(GfxTextureHandle handl)
	{
		GfxTexture* pTexture = ResolveTexture(handl);
		return pTexture ? Gfx::GetTextureShaderView(pTexture) : ~0u;
	}

	u32 GraphicsModule::GetTextureAccessView(GfxTextureHandle handl)
	{
		GfxTexture* pTexture = ResolveTexture(handl);
		return pTexture ? Gfx::GetTextureAccessView(pTexture) : ~0u;
	}

	u32 GraphicsModule::GetBufferShaderView(GfxBufferHandle handl)
	{
		GfxBuffer* pBuffer = ResolveBuffer(handl);
		return pBuffer ? Gfx::GetBufferShaderView(pBuffer) : ~0u;
	}

	u32 GraphicsModule::GetBufferAccessView(GfxBufferHandle handl)
	{
		GfxBuffer* pBuffer = ResolveBuffer(handl);
		return pBuffer ? Gfx::GetBufferAccessView(pBuffer) : ~0u;
	}

	void GraphicsModule::BeginFrame()
	{
		const u32 slot = GetFrameSlot();

		Gfx::WaitGfxQueueCPU(m_graphicsQueue, m_slotValues[slot]);

		for (CmdLane& lane : m_lanes)
		{
			if (lane.pAllocators[slot] != nullptr)
				Gfx::ResetGfxCmdAllocator(lane.pAllocators[slot]);
			lane.nextLocal[slot] = 0;
		}
	}

	void GraphicsModule::EndFrame()
	{
		const u32 slot = GetFrameSlot();

		m_slotValues[slot] = Gfx::SignalGfxQueue(m_graphicsQueue);
		Gfx::Present(m_swapchain);
		m_frameIndex++;
	}

	GfxTexture* GraphicsModule::GetCurrentBackbuffer()
	{
		return Gfx::RequestTexture(m_swapchain, Gfx::GetBackbufferIndex(m_swapchain));
	}

	GfxCmdList* GraphicsModule::RequestCmdList(GfxQueueType type)
	{
		const u32 slot = GetFrameSlot();
		const u32 lane = ThreadPool::LocalWorkerIndex() * QueueTypeCount + u32(type);

		CmdLane& cmdLane = m_lanes[lane];

		if (cmdLane.pAllocators[slot] == nullptr)
			cmdLane.pAllocators[slot] = Gfx::CreateGfxCmdAllocator(m_mainDevice, type);

		GfxCmdList* pCmd = nullptr;
		const u32 local = cmdLane.nextLocal[slot]++;

		if (local < cmdLane.lists[slot].size())
			pCmd = cmdLane.lists[slot][local];
		else
		{
			pCmd = Gfx::CreateGfxCmdList(m_mainDevice, cmdLane.pAllocators[slot]);
			cmdLane.lists[slot].push_back(pCmd);
		}

		Gfx::BeginGfxCmdList(pCmd, cmdLane.pAllocators[slot]);

		if (type != GfxQueueType::Transfer)
			Gfx::CmdSetupBindless(pCmd, m_globalLayout, m_resourceHeap);

		return pCmd;
	}

	void GraphicsModule::SubmitCmdLists(GfxCmdList* const* ppLists, u32 count, GfxQueueType type)
	{
		Gfx::ExecuteGfxCmdLists(GetQueue(type), ppLists, count);
	}

	GfxTextureHandle GraphicsModule::CreateTexture(const GfxTextureDesc& desc)
	{
		GfxTexture* pTexture = Gfx::CreateGfxTexture(m_mainDevice, desc);
		if (pTexture == nullptr)
			return GfxTextureHandle();

		if (has(desc.usage, GfxTextureUsage::Sampled))
			Gfx::CreateTextureSRV(m_mainDevice, m_resourceHeap, pTexture);
		if (has(desc.usage, GfxTextureUsage::RenderTarget))
			Gfx::CreateTextureRTV(m_mainDevice, m_colorHeap, pTexture);
		if (has(desc.usage, GfxTextureUsage::DepthStencil))
			Gfx::CreateTextureDSV(m_mainDevice, m_depthHeap, pTexture);
		if (has(desc.usage, GfxTextureUsage::Storage))
			Gfx::CreateTextureUAV(m_mainDevice, m_resourceHeap, pTexture);

		return m_texturePool.Insert(std::move(pTexture));
	}

	void GraphicsModule::DestroyTexture(GfxTextureHandle handl)
	{
		m_texturePool.ResolveWrite(handl, [&](GfxTexture*& pTexture)
			{
				const u32 srv = Gfx::GetTextureShaderView(pTexture);
				const u32 uav = Gfx::GetTextureAccessView(pTexture);
				const u32 rtv = Gfx::GetTextureTargetView(pTexture);
				const u32 dsv = Gfx::GetTextureDepthView(pTexture);

				if (srv != ~0u)
					Gfx::FreeDescriptorIndex(m_resourceHeap, srv);
				if (uav != ~0u)
					Gfx::FreeDescriptorIndex(m_resourceHeap, uav);
				if (rtv != ~0u)
					Gfx::FreeDescriptorIndex(m_colorHeap, rtv);
				if (dsv != ~0u)
					Gfx::FreeDescriptorIndex(m_depthHeap, dsv);

				Gfx::DestroyGfxTexture(pTexture);
			});

		m_texturePool.Remove(handl);
	}

	GfxBufferHandle GraphicsModule::CreateBuffer(const GfxBufferDesc& desc)
	{
		GfxBuffer* pBuffer = Gfx::CreateGfxBuffer(m_mainDevice, desc);
		if (pBuffer == nullptr)
			return GfxBufferHandle();

		const b8 shaderRead = has(desc.usage, GfxBufferUsage::Vertex)
			|| has(desc.usage, GfxBufferUsage::Index)
			|| has(desc.usage, GfxBufferUsage::Storage);

		if (shaderRead && desc.memory == GfxMemoryType::GPU)
			Gfx::CreateBufferSRV(m_mainDevice, m_resourceHeap, pBuffer);
		if (has(desc.usage, GfxBufferUsage::Storage))
			Gfx::CreateBufferUAV(m_mainDevice, m_resourceHeap, pBuffer);

		return m_bufferPool.Insert(std::move(pBuffer));
	}

	void GraphicsModule::WriteBuffer(GfxBufferHandle handl, const void* pData, usize sizeInBytes, usize offset)
	{
		m_bufferPool.ResolveWrite(handl, [&](GfxBuffer*& pBuffer)
			{
				Gfx::WriteGfxBuffer(pBuffer, pData, sizeInBytes, offset);
			});
	}

	void GraphicsModule::DestroyBuffer(GfxBufferHandle handl)
	{
		m_bufferPool.ResolveWrite(handl, [&](GfxBuffer*& pBuffer)
			{
				const u32 srv = Gfx::GetBufferShaderView(pBuffer);
				const u32 uav = Gfx::GetBufferAccessView(pBuffer);

				if (srv != ~0u)
					Gfx::FreeDescriptorIndex(m_resourceHeap, srv);
				if (uav != ~0u)
					Gfx::FreeDescriptorIndex(m_resourceHeap, uav);

				Gfx::DestroyGfxBuffer(pBuffer);
			});

		m_bufferPool.Remove(handl);
	}

	GfxPipelineHandle GraphicsModule::CreatePipeline(const GfxGraphicsPipelineDesc& desc)
	{
		GfxPipeline* pPipeline = Gfx::CreateGfxGraphicsPipeline(m_mainDevice, m_globalLayout, desc);
		if (pPipeline == nullptr)
			return GfxPipelineHandle();

		return m_pipelinePool.Insert(std::move(pPipeline));
	}

	GfxPipelineHandle GraphicsModule::CreatePipeline(const GfxComputePipelineDesc& desc)
	{
		GfxPipeline* pPipeline = Gfx::CreateGfxComputePipeline(m_mainDevice, m_globalLayout, desc);
		if (pPipeline == nullptr)
			return GfxPipelineHandle();

		return m_pipelinePool.Insert(std::move(pPipeline));
	}

	void GraphicsModule::DestroyPipeline(GfxPipelineHandle handl)
	{
		m_pipelinePool.ResolveWrite(handl, [&](GfxPipeline*& pPipeline)
			{
				Gfx::DestroyGfxPipeline(pPipeline);
			});

		m_pipelinePool.Remove(handl);
	}

	void GraphicsModule::OnDetach()
	{
		const u64 flushValue = Gfx::SignalGfxQueue(m_graphicsQueue);
		Gfx::WaitGfxQueueCPU(m_graphicsQueue, flushValue);

		Gfx::ShutdownGfxImGui();

		for (CmdLane& lane : m_lanes)
		{
			for (u32 slot = 0; slot < MaxFramesInFlight; slot++)
			{
				for (GfxCmdList* pCmd : lane.lists[slot])
					Gfx::DestroyGfxCmdList(pCmd);

				if (lane.pAllocators[slot] != nullptr)
					Gfx::DestroyGfxCmdAllocator(lane.pAllocators[slot]);
			}
		}
		m_lanes.clear();

		Gfx::DestroyGfxSwapchain(m_swapchain);

		Gfx::DestroyGfxQueue(m_graphicsQueue);
		Gfx::DestroyGfxQueue(m_computeQueue);
		Gfx::DestroyGfxQueue(m_transferQueue);

		Gfx::DestroyGfxDescriptorHeap(m_resourceHeap);
		Gfx::DestroyGfxDescriptorHeap(m_colorHeap);
		Gfx::DestroyGfxDescriptorHeap(m_depthHeap);

		Gfx::DestroyGfxPipelineLayout(m_globalLayout);

		Gfx::ShutdownGfxDevice(m_mainDevice);
	}
}