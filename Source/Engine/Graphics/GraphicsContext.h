#pragma once

#include <Engine/Core/Context.h>
#include <Runtime/RHI/Device/GfxDevice.h>
#include <Runtime/RHI/Queue/GfxQueue.h>
#include <Runtime/RHI/Fence/GfxFence.h>
#include <Runtime/RHI/Descriptor/GfxDescriptorHeap.h>
#include <Runtime/RHI/Command/GfxCommandList.h>
#include <Runtime/RHI/Swapchain/GfxSwapchain.h>

namespace Horizon::Engine
{
	class H_EXPORT GraphicsContext : public Context
	{
	public:
		static constexpr u32 MaxFramesInFlight = 2;

	public:
		GraphicsContext() = default;
		~GraphicsContext() = default;

		RHI::GfxDevice* GetDevice() const { return m_device; }

		RHI::GfxQueue* GetGraphicsQueue() const { return m_graphicsQueue; }
		RHI::GfxQueue* GetComputeQueue() const { return m_computeQueue; }
		RHI::GfxQueue* GetTransferQueue() const { return m_transferQueue; }
		
		RHI::GfxSwapchain* GetSwapchain() const { return m_swapchain; }

		ModuleReport OnInitialize() final;
		void OnFinalize() final;
		void DeclareDependencies(ModuleGraph& graph) final;

	private:
		RHI::GfxDevice* m_device = nullptr;

		RHI::GfxQueue* m_graphicsQueue = nullptr;
		RHI::GfxQueue* m_computeQueue = nullptr;
		RHI::GfxQueue* m_transferQueue = nullptr;

		RHI::GfxSwapchain* m_swapchain = nullptr;
	};
}