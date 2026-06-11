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
		{
			GfxTexture* pImage = Gfx::RequestTexture(m_swapchain, i);
			Gfx::CreateTextureRTV(m_mainDevice, m_colorHeap, pImage);
		}

		m_globalLayout = Gfx::CreateGfxGlobalPipelineLayout(m_mainDevice);

		auto* pJobModule = engine.TryGetModule<JobModule>();

		Gfx::InitGfxImGui(m_mainDevice, m_graphicsQueue, m_resourceHeap,
			currWindow.GetAPIHandle(), GfxTextureFormat::RGBA8, MaxFramesInFlight);
	}

	GfxQueue* GraphicsModule::GetQueue(GfxQueueType type) const
	{
		return type == GfxQueueType::Compute ? m_computeQueue :
			type == GfxQueueType::Transfer ? m_transferQueue : m_graphicsQueue;
	}

	u32 GraphicsModule::GetTextureShaderView(GfxTextureHandle handl)
	{
		return 0;
	}

	u32 GraphicsModule::GetTextureAccessView(GfxTextureHandle handl)
	{
		return 0;
	}

	u32 GraphicsModule::GetBufferShaderView(GfxBufferHandle handl)
	{
		return 0;
	}

	u32 GraphicsModule::GetBufferAccessView(GfxBufferHandle handl)
	{
		return 0;
	}

	void GraphicsModule::BeginFrame()
	{
		
	}

	void GraphicsModule::EndFrame()
	{
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

		return GfxTextureHandle();
	}

	void GraphicsModule::DestroyTexture(GfxTextureHandle handl)
	{
		
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

		return GfxBufferHandle();
	}

	void GraphicsModule::WriteBuffer(GfxBufferHandle handl, const void* pData, usize sizeInBytes, usize offset)
	{
	}

	void GraphicsModule::DestroyBuffer(GfxBufferHandle handl)
	{
	}

	GfxPipelineHandle GraphicsModule::CreatePipeline(const GfxGraphicsPipelineDesc& desc)
	{
		GfxPipeline* pPipeline = Gfx::CreateGfxGraphicsPipeline(m_mainDevice, m_globalLayout, desc);
		if (pPipeline == nullptr)
			return GfxPipelineHandle();

		return GfxPipelineHandle();
	}

	GfxPipelineHandle GraphicsModule::CreatePipeline(const GfxComputePipelineDesc& desc)
	{
		GfxPipeline* pPipeline = Gfx::CreateGfxComputePipeline(m_mainDevice, m_globalLayout, desc);
		if (pPipeline == nullptr)
			return GfxPipelineHandle();

		return GfxPipelineHandle();
	}

	void GraphicsModule::DestroyPipeline(GfxPipelineHandle handl)
	{
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