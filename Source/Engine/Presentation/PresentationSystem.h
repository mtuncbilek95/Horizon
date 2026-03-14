#pragma once

#include <Engine/System/System.h>
#include <Engine/Presentation/PresentRequest.h>

namespace Horizon
{
	class GfxSwapchain;
	class GfxQueue;

	class PresentationSystem : public System<PresentationSystem>
	{
	public:
		PresentationSystem() = default;
		virtual ~PresentationSystem() = default;

		SystemReport OnInitialize() override;
		void OnSync() override;
		void OnFinalize() override;

		void RequestPresent(const PresentRequest& request);

	private:
		GfxSwapchain* m_swapchain;
		GfxQueue* m_presentationQueue;

		std::vector<PresentRequest> m_pendingPresentRequests;
	};
}
