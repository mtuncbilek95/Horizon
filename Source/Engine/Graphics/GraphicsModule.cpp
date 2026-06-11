#include "GraphicsModule.h"

#include <Engine/Engine/Engine.h>
#include <Engine/Window/WindowModule.h>

#include <cmath>

#include <imgui.h>

namespace Horizon
{
	inline constexpr u32 MaxBindlessCapacity = 1 << 16;
	inline constexpr u32 MaxFramesInFlight = 2;

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

		Gfx::InitGfxImGui(m_mainDevice, m_graphicsQueue, m_resourceHeap,
			currWindow.GetAPIHandle(), GfxTextureFormat::RGBA8, MaxFramesInFlight);
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