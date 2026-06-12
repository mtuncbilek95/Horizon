#include "GraphicsModule.h"

#include <Engine/Engine/Engine.h>
#include <Engine/Window/WindowModule.h>
#include <Engine/Job/JobModule.h>

#include <cmath>

#include <imgui.h>

namespace Horizon
{
	GfxQueue* GraphicsModule::GetQueue(GfxQueueType type) const
	{
		return type == GfxQueueType::Compute ? m_computeQueue :
			type == GfxQueueType::Transfer ? m_transferQueue : m_graphicsQueue;
	}

	GfxDescriptorHeap* GraphicsModule::GetDescriptorHeap(GfxDescriptorHeapType type) const
	{
		return type == GfxDescriptorHeapType::Color ? m_colorHeap :
			type == GfxDescriptorHeapType::Resource ? m_resourceHeap : m_depthHeap;
	}

	GfxTextureHandle GraphicsModule::CreateTexture(const GfxTextureDesc& desc)
	{
		GfxTexture* pTexture = Gfx::CreateGfxTexture(m_mainDevice, desc);
		if (pTexture == nullptr)
			return GfxTextureHandle();

		u32 srvIndex = kInvalid32, uavIndex = kInvalid32, rtvIndex = kInvalid32, dsvIndex = kInvalid32;
		if (has(desc.usage, GfxTextureUsage::Sampled))
			srvIndex = Gfx::CreateTextureSRV(m_mainDevice, m_resourceHeap, pTexture);
		if (has(desc.usage, GfxTextureUsage::RenderTarget))
			rtvIndex = Gfx::CreateTextureRTV(m_mainDevice, m_colorHeap, pTexture);
		if (has(desc.usage, GfxTextureUsage::DepthStencil))
			dsvIndex = Gfx::CreateTextureDSV(m_mainDevice, m_depthHeap, pTexture);
		if (has(desc.usage, GfxTextureUsage::Storage))
			uavIndex = Gfx::CreateTextureUAV(m_mainDevice, m_resourceHeap, pTexture);

		TextureRecord record =
		{
			.pTex = pTexture,
			.desc = desc,
			.currState = GfxResourceState::Common,
			.shaderView = srvIndex, .accessView = uavIndex,
			.colorView = rtvIndex, .depthView = dsvIndex
		};
		return m_texturePool.Create(record);
	}

	u32 GraphicsModule::GetTextureShaderView(GfxTextureHandle handl)
	{
		auto* record = m_texturePool.Resolve(handl);
		return record->shaderView;
	}

	u32 GraphicsModule::GetTextureAccessView(GfxTextureHandle handl)
	{
		auto* record = m_texturePool.Resolve(handl);
		return record->accessView;
	}

	u32 GraphicsModule::GetTextureColorView(GfxTextureHandle handl)
	{
		auto* record = m_texturePool.Resolve(handl);
		return record->colorView;
	}

	u32 GraphicsModule::GetTextureDepthView(GfxTextureHandle handl)
	{
		auto* record = m_texturePool.Resolve(handl);
		return record->depthView;
	}

	void GraphicsModule::DestroyTexture(GfxTextureHandle handl)
	{
		m_texturePool.DestroyWith(handl, [&](TextureRecord* record)
			{
				if (record->accessView != kInvalid32)
					Gfx::FreeDescriptorIndex(m_resourceHeap, record->accessView);

				if (record->shaderView != kInvalid32)
					Gfx::FreeDescriptorIndex(m_resourceHeap, record->shaderView);

				if (record->colorView != kInvalid32)
					Gfx::FreeDescriptorIndex(m_colorHeap, record->colorView);

				if (record->depthView != kInvalid32)
					Gfx::FreeDescriptorIndex(m_depthHeap, record->depthView);

				Gfx::DestroyGfxTexture(record->pTex);
			}
		);
	}

	GfxTexture* GraphicsModule::GetTexture(GfxTextureHandle handl)
	{
		auto* record = m_texturePool.Resolve(handl);
		if (!record)
			return nullptr;

		return record->pTex;
	}

	void GraphicsModule::BeginFrame()
	{
		u32 slot = u32(m_frameIndex % MaxFramesInFlight);

		Gfx::WaitGfxQueueCPU(m_graphicsQueue, m_frameFenceValues[slot]);
		Gfx::ResetGfxCmdAllocator(m_frameAllocators[slot]);

		u32 backBufferIndex = Gfx::GetBackbufferIndex(m_swapchain);
		m_currPresentImg = Gfx::RequestTexture(m_swapchain, backBufferIndex);

		Gfx::BeginGfxCmdList(m_frameCmd, m_frameAllocators[slot]);
		Gfx::CmdSetupBindless(m_frameCmd, m_globalLayout, m_resourceHeap);
	}

	void GraphicsModule::EndFrame()
	{
		Gfx::CloseGfxCmdList(m_frameCmd);
		Gfx::ExecuteGfxCmdLists(m_graphicsQueue, &m_frameCmd, 1);
		Gfx::Present(m_swapchain);

		u32 slot = u32(m_frameIndex % MaxFramesInFlight);
		m_frameFenceValues[slot] = Gfx::SignalGfxQueue(m_graphicsQueue);
		m_frameIndex++;
	}

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

		for (u32 i = 0; i < MaxFramesInFlight; i++)
			m_frameAllocators[i] = Gfx::CreateGfxCmdAllocator(m_mainDevice, GfxQueueType::Graphics);

		m_frameCmd = Gfx::CreateGfxCmdList(m_mainDevice, m_frameAllocators[0]);
	}

	void GraphicsModule::OnSync()
	{
		BeginFrame();
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