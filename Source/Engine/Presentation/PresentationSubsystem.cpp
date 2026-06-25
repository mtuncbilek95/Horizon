#include "PresentationSubsystem.h"

#include <Engine/Core/Engine.h>
#include <Engine/Window/WindowSubsystem.h>
#include <Engine/Graphics/GraphicsSubsystem.h>

#include <Runtime/PAL/Window/Window.h>
#include <Runtime/RHI/Device/GfxDevice.h>
#include <Runtime/RHI/Swapchain/GfxSwapchain.h>

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

		return EngineReport();
	}

	void PresentationSubsystem::OnDetach()
	{
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
}