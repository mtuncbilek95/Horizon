#pragma once

#include <Engine/Engine/IModule.h>
#include <Runtime/Graphics/RHI/Swapchain/GfxSwapchain.h>
#include <Runtime/Graphics/RHI/Command/GfxCommandList.h>

#include <memory>

namespace Horizon
{
	class GfxDevice;
	class GfxQueue;
	class FrameContext;

	class PresentModule : public IModule
	{
	public:
		void OnAttach(Engine& engine) final;
		void OnSync() final;
		void OnDetach() final;

	private:
		GfxDevice* m_device = nullptr;
		GfxQueue* m_graphicsQueue = nullptr;
		FrameContext* m_frameContext = nullptr;

		std::unique_ptr<GfxSwapchain> m_swapchain;
		std::unique_ptr<GfxCommandList> m_uiCommandList;

		u32 m_width = 0;
		u32 m_height = 0;
	};
}
