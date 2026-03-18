#include "PresentationSystem.h"

#include <Runtime/Graphics/RHI/Device/GfxDevice.h>
#include <Runtime/Graphics/RHI/Swapchain/GfxSwapchain.h>

#include <Engine/Window/WindowSystem.h>
#include <Engine/Graphics/GraphicsSystem.h>

namespace Horizon
{
    PresentationSystem::PresentationSystem()
    {
    }

    EngineReport PresentationSystem::OnInitialize()
    {
        auto& windowSystem = RequestSystem<WindowSystem>();
        auto& graphicsSystem = RequestSystem<GraphicsSystem>();

        m_swapchain = graphicsSystem.Device().CreateSwapchain(
            GfxSwapchainDesc()
            .setImageSize(windowSystem.WindowSize())
            .setImageCount(2)
            .setFormat(ImageFormat::B8G8R8A8_UNorm)
            .setPresentMode(PresentMode::Fifo)
            .setGraphicsQueue(&graphicsSystem.GraphicsQueue())
            .setWindowHandler(windowSystem.Handle())
            .setWindowInstance(windowSystem.Instance())
            .setWindowAPI(windowSystem.APIWindow())
        );

        return EngineReport();
    }

    void PresentationSystem::OnSync()
    {
    }

    void PresentationSystem::OnFinalize()
    {
    }
}