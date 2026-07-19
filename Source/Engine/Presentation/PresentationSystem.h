#pragma once

#include <Engine/Core/System.h>

namespace Horizon
{
	class GfxSwapchain;
	class GfxTexture;
	class GfxFence;
	class GfxQueue;

	class H_EXPORT PresentationSystem final : public System
	{
	public:
		PresentationSystem() = default;
		~PresentationSystem() = default;

		EngineReport OnAttach(Engine* engine) final;
		void OnDetach() final;

		void GetInitializeOrder(OrderRules& rules) const final;
		void GetExecutionOrder(OrderRules& rules) const final;

		i8 AcquireImageIndex();
		GfxTexture* GetBackbuffer(u8 index) const;
		void Present(u8 index);
		void WaitIdle();

	private:
		GfxSwapchain* m_swapchain = nullptr;
		GfxQueue* m_graphicsQueue = nullptr;
		GfxFence* m_frameFence = nullptr;

		u32 m_imageCount = 0;
		u64 m_imageFenceValues[8] = {};
	};
}