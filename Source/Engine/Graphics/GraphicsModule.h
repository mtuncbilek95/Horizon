#pragma once

#include <Engine/Engine/IModule.h>
#include <Runtime/Graphics/RHI/Device/GfxDevice.h>
#include <Runtime/Graphics/RHI/Queue/GfxQueue.h>
#include <Runtime/Graphics/RHI/Swapchain/GfxSwapchain.h>

namespace Horizon
{
	class GraphicsModule final : public IModule
	{
	public:
		void OnAttach(Engine& engine) final;
		void OnSync() final;
		void OnDetach() final;

		GfxDevice* GetDevice() const { return &*m_device; }
		GfxQueue* GetGraphicsQueue() const { return &*m_graphicsQueue; }
		GfxQueue* GetComputeQueue() const { return &*m_graphicsQueue; }
		GfxQueue* GetTransferQueue() const { return &*m_graphicsQueue; }

	private:
		std::unique_ptr<GfxDevice> m_device;

		std::unique_ptr<GfxQueue> m_graphicsQueue;
		std::unique_ptr<GfxQueue> m_computeQueue;
		std::unique_ptr<GfxQueue> m_transferQueue;
	};
}