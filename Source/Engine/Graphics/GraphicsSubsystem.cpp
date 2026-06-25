#include "GraphicsSubsystem.h"

#include <Engine/Core/Engine.h>
#include <Engine/Window/WindowSubsystem.h>

#include <Runtime/PAL/Window/Window.h>
#include <Runtime/RHI/Device/GfxDevice.h>
#include <Runtime/RHI/Queue/GfxQueue.h>
#include <Runtime/RHI/Fence/GfxFence.h>
#include <Runtime/RHI/Command/GfxCommandList.h>
#include <Runtime/RHI/Swapchain/GfxSwapchain.h>
#include <Runtime/Definitions/Allocator.h>

namespace Horizon
{
	void GraphicsSubsystem::OnAttach(Engine* pEngine)
	{
		Subsystem::OnAttach(pEngine);

		auto* pWindowSub = m_engine->TryGetModule<WindowSubsystem>();

		m_device = CreateGfxDevice();

		m_graphicsQueue = m_device->CreateQueue(GfxQueueType::Graphics);
		m_computeQueue = m_device->CreateQueue(GfxQueueType::Compute);
		m_transferQueue = m_device->CreateQueue(GfxQueueType::Transfer);

		WindowRect windowRect = pWindowSub->GetWindow()->GetRect();

		GfxSwapchainDesc swapDesc = {};
		swapDesc.pWindowHandle = (void*)pWindowSub->GetWindow()->GetOSHandle();
		swapDesc.imageCount = 3;
		swapDesc.width = windowRect.width;
		swapDesc.height = windowRect.height;
		swapDesc.vSync = true;
		swapDesc.bAllowTearing = false;
		m_swapchain = m_device->CreateSwapchain(swapDesc, m_graphicsQueue);
	}

	void GraphicsSubsystem::OnSync()
	{
	}

	void GraphicsSubsystem::OnDetach()
	{
		Allocator::Delete(m_swapchain);

		Allocator::Delete(m_graphicsQueue);
		Allocator::Delete(m_computeQueue);
		Allocator::Delete(m_transferQueue);

		Allocator::Delete(m_device);
	}

	void GraphicsSubsystem::GetExecuteAfter(std::vector<std::type_index>& out) const
	{
		Requires<WindowSubsystem>(out);
	}

	void GraphicsSubsystem::GetExecuteBefore(std::vector<std::type_index>& out) const
	{
	}
}
