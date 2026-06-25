#pragma once

#include <Engine/Core/Subsystem.h>

#include <Runtime/RHI/GfxTypes.h>

namespace Horizon
{
	class GfxDevice;
	class GfxQueue;
	class GfxSwapchain;
	class GfxFence;
	class GfxCommandList;

	class GraphicsSubsystem final : public Subsystem
	{
	public:
		GraphicsSubsystem() = default;
		~GraphicsSubsystem() = default;

		void OnAttach(Engine* engine) final;
		void OnSync() final;
		void OnDetach() final;

		void GetExecuteAfter(std::vector<std::type_index>& out) const final;
		void GetExecuteBefore(std::vector<std::type_index>& out) const final;

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
