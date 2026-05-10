#pragma once

#include <Engine/Core/System.h>
#include <Engine/Presentation/PresentationObjects.h>

namespace Horizon
{
	class GfxSwapchain;
	class GfxSemaphore;
	class GfxFence;
	class GfxQueue;

	class PresentationSystem : public System<PresentationSystem>
	{
	public:
		static constexpr u32 MaxFramesInFlight = 2;

	public:
		GfxSwapchain& Swapchain() const { return *m_swapchain.get(); }

		CompositePresentObject AcquireNextImage();
		void SubmitPresent(const CompositeSubmitObject& obj);

	private:
		EngineReport OnInitialize() final;
		void OnSync() final;
		void OnFinalize() final;

	private:
		GfxQueue* m_graphicsQueue;
		std::shared_ptr<GfxSwapchain> m_swapchain;

		std::vector<std::shared_ptr<GfxSemaphore>> m_imageAvailableSemaphores;
		std::vector<std::shared_ptr<GfxSemaphore>> m_renderFinishedSemaphores;
		std::vector<std::shared_ptr<GfxFence>> m_fences;

		u32 m_frameIndex = 0;
	};
}