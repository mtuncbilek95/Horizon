#include "PresentationSubsystem.h"

#include <Engine/Core/Engine.h>
#include <Engine/Window/WindowSubsystem.h>
#include <Engine/Graphics/GraphicsSubsystem.h>

#include <Runtime/PAL/Window/Window.h>
#include <Runtime/RHI/Device/GfxDevice.h>
#include <Runtime/RHI/Swapchain/GfxSwapchain.h>
#include <Runtime/RHI/Queue/GfxQueue.h>
#include <Runtime/RHI/Fence/GfxFence.h>

namespace Horizon
{
	EngineReport PresentationSubsystem::OnAttach(Engine* engine)
	{
		Subsystem::OnAttach(engine);

		auto* pWindowSub = m_engine->TryGetSubsystem<WindowSubsystem>();
		if (!pWindowSub)
			return EngineReport("Failed to get WindowSubsystem. Nothing will work...");

		auto* pGraphSub = m_engine->TryGetSubsystem<GraphicsSubsystem>();
		if (!pGraphSub)
			return EngineReport("Failed to get GraphicsSubsystem. Nothing will work...");

		PAL::WindowRect windowRect = pWindowSub->GetWindow()->GetRect();

		GfxSwapchainDesc swapDesc = {};
		swapDesc.pWindowHandle = (void*)pWindowSub->GetWindow()->GetOSHandle();
		swapDesc.imageCount = 3;
		swapDesc.width = windowRect.width;
		swapDesc.height = windowRect.height;
		swapDesc.vSync = true;
		swapDesc.bAllowTearing = false;
		m_swapchain = pGraphSub->GetDevice()->CreateSwapchain(swapDesc, pGraphSub->GetGraphicsQueue());
		if (!m_swapchain)
			return EngineReport("Failed to create GfxSwapchain");

		m_frameFence = pGraphSub->GetDevice()->CreateFence();
		if (!m_frameFence)
			return EngineReport("Failed to create present fence");

		m_imageCount = swapDesc.imageCount;

		Terminal::Debug("PresentationSubsystem", "Swapchain has been initialized!");
		return EngineReport();
	}

	void PresentationSubsystem::OnDetach()
	{
		Allocator::Delete(m_frameFence);
		Allocator::Delete(m_swapchain);
	}

	void PresentationSubsystem::GetInitializeOrder(OrderRules& rules) const
	{
		Requires<WindowSubsystem, GraphicsSubsystem>(rules.after);
	}

	void PresentationSubsystem::GetExecutionOrder(OrderRules& rules) const
	{
		rules.tier = OrderTier::Last;
	}

	i8 PresentationSubsystem::AcquireImageIndex()
	{
		if (!m_swapchain)
			return -1;

		u32 index = m_swapchain->GetCurrentIndex();

		m_frameFence->WaitCPU(m_imageFenceValues[index]);

		return i8(index);
	}

	GfxTexture* PresentationSubsystem::GetBackbuffer(u8 index) const
	{
		return m_swapchain->GetBackbuffer(index);
	}

	void PresentationSubsystem::Present(u8 index)
	{
		m_swapchain->Present();
		m_imageFenceValues[index] = m_graphicsQueue->Signal(m_frameFence);
	}
}