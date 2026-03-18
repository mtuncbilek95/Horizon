#include "GraphicsSystem.h"

#include <Runtime/Graphics/RHI/Command/GfxCommandBuffer.h>
#include <Runtime/Graphics/RHI/Command/GfxCommandPool.h>
#include <Runtime/Graphics/RHI/Device/GfxDevice.h>
#include <Runtime/Graphics/RHI/Instance/GfxInstance.h>
#include <Runtime/Graphics/RHI/Queue/GfxQueue.h>
#include <Runtime/Graphics/RHI/Swapchain/GfxSwapchain.h>

#include <Engine/Window/WindowSystem.h>

namespace Horizon
{
    GraphicsSystem::GraphicsSystem()
    {

    }
    EngineReport GraphicsSystem::OnInitialize()
    {
        auto& windowSystem = RequestSystem<WindowSystem>();

        m_instance = GfxInstance::Create(
            GfxInstanceDesc()
            .setAppName("Horizon")
            .setAppVersion({ 1, 0, 0 })
            .setAPIType(GfxType::Vulkan)
        );
        m_device = m_instance->CreateDevice(
            GfxDeviceDesc()
            .setGraphicsQueueCount(1)
            .setComputeQueueCount(1)
            .setTransferQueueCount(1)
        );
        m_graphicsQueue = m_device->CreateQueue(QueueType::Graphics);
        m_computeQueue = m_device->CreateQueue(QueueType::Compute);
        m_transferQueue = m_device->CreateQueue(QueueType::Transfer);

        return EngineReport();
    }

    void GraphicsSystem::OnSync()
    {
    }

    void GraphicsSystem::OnFinalize()
    {
    }
}