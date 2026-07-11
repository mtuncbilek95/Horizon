#pragma once

#include <Engine/Core/System.h>

#include <Runtime/RHI/GfxTypes.h>

namespace Horizon
{
	class GfxDevice;
	class GfxQueue;
	class GfxSwapchain;
	class GfxFence;
	class GfxCommandList;

	class H_EXPORT GraphicsSystem final : public System
	{
	public:
		GraphicsSystem() = default;
		~GraphicsSystem() = default;

		SystemReport OnAttach(Engine* engine) final;
		void OnDetach() final;

		void GetInitializeOrder(OrderRules& rules) const final;
		void GetExecutionOrder(OrderRules& rules) const final;

		GfxDevice* GetDevice() const { return m_device; }
		GfxQueue* GetGraphicsQueue() const { return m_graphicsQueue; }
		GfxQueue* GetComputeQueue() const { return m_computeQueue; }
		GfxQueue* GetTransferQueue() const { return m_transferQueue; }
		GfxSwapchain* GetSwapchain() const { return m_swapchain; }

	private:
		GfxDevice* m_device = nullptr;

		GfxQueue* m_graphicsQueue = nullptr;
		GfxQueue* m_computeQueue = nullptr;
		GfxQueue* m_transferQueue = nullptr;

		GfxSwapchain* m_swapchain = nullptr;
	};
}
