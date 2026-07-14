#include "PresentationSystem.h"

#include <Engine/Core/Engine.h>
#include <Engine/Window/WindowSystem.h>
#include <Engine/Graphics/GraphicsContext.h>

#include <Runtime/PAL/Window/Window.h>
#include <Runtime/RHI/Device/GfxDevice.h>
#include <Runtime/RHI/Swapchain/GfxSwapchain.h>
#include <Runtime/RHI/Queue/GfxQueue.h>
#include <Runtime/RHI/Fence/GfxFence.h>

namespace Horizon
{
	EngineReport PresentationSystem::OnAttach(Engine* engine)
	{
		System::OnAttach(engine);

		auto* pWindowSub = m_engine->TryGetSystem<WindowSystem>();
		if (!pWindowSub)
			return EngineReport("Failed to get WindowSystem. Nothing will work...");

		auto* pGraphSub = m_engine->TryGetContext<GraphicsContext>();
		if (!pGraphSub)
			return EngineReport("Failed to get GraphicsContext. Nothing will work...");

		m_graphicsQueue = pGraphSub->GetGraphicsQueue();

		PAL::WindowRect windowRect = pWindowSub->GetWindow()->GetRect();

		GfxSwapchainDesc swapDesc = {};
		swapDesc.pWindowHandle = (void*)pWindowSub->GetWindow()->GetOSHandle();
		swapDesc.imageCount = 3;
		swapDesc.width = windowRect.width;
		swapDesc.height = windowRect.height;
		swapDesc.vSync = false;
		swapDesc.bAllowTearing = false;
		m_swapchain = pGraphSub->GetDevice()->CreateSwapchain(swapDesc, pGraphSub->GetGraphicsQueue());
		if (!m_swapchain)
			return EngineReport("Failed to create GfxSwapchain");

		m_frameFence = pGraphSub->GetDevice()->CreateFence();
		if (!m_frameFence)
			return EngineReport("Failed to create present fence");

		m_imageCount = swapDesc.imageCount;

		Terminal::Debug("PresentationSystem", "Swapchain has been initialized!");
		return EngineReport();
	}

	void PresentationSystem::OnDetach()
	{
		Allocator::Delete(m_frameFence);
		Allocator::Delete(m_swapchain);
	}

	void PresentationSystem::GetInitializeOrder(OrderRules& rules) const
	{
		Requires<WindowSystem>(rules.after);
	}

	void PresentationSystem::GetExecutionOrder(OrderRules& rules) const
	{
		rules.tier = OrderTier::Last;
	}

	i8 PresentationSystem::AcquireImageIndex()
	{
		if (!m_swapchain)
			return -1;

		u32 index = m_swapchain->GetCurrentIndex();

		m_frameFence->WaitCPU(m_imageFenceValues[index]);

		return i8(index);
	}

	GfxTexture* PresentationSystem::GetBackbuffer(u8 index) const
	{
		return m_swapchain->GetBackbuffer(index);
	}

	void PresentationSystem::Present(u8 index)
	{
		m_swapchain->Present();
		m_imageFenceValues[index] = m_graphicsQueue->Signal(m_frameFence);
	}
}