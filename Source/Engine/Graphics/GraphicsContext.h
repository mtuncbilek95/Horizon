#pragma once

#include <Engine/Core/Context.h>
#include <Runtime/RHI/GfxTypes.h>

namespace Horizon
{
	class GfxDevice;
	class GfxQueue;
	class GfxSwapchain;
	class GfxFence;
	class GfxCommandList;
}

namespace Horizon::Engine
{
	class H_EXPORT GraphicsContext final : public Context
	{
	public:
		GraphicsContext() = default;
		~GraphicsContext() = default;

		AppReport OnAttach(Application* engine) final;
		void OnDetach() final;

		void GetInitializeOrder(OrderRules& rules) const final;

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
