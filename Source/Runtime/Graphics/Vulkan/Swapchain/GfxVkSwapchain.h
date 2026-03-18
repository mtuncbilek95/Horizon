#pragma once

#include <Runtime/Graphics/RHI/Swapchain/GfxSwapchain.h>

#include <vulkan/vulkan.h>

#include <vector>

namespace Horizon
{
    class GfxVkSwapchain final : public GfxSwapchain
    {
    public:
        GfxVkSwapchain(const GfxSwapchainDesc& desc, GfxDevice* pDevice);
        ~GfxVkSwapchain() override;

        u32 AcquireNextImage(GfxSemaphore* signal, GfxFence* fence) final;
        void Present(const std::vector<GfxSemaphore*>& waits) const final;
        void Resize(const Math::Vec2u& newSize) final;
        
    private:
        VkSwapchainKHR m_swapchain;
        VkSurfaceKHR m_surface;

        VkFence m_barrierFence;
        VkCommandPool m_barrierPool;
        VkCommandBuffer m_barrierBuffer;

        u32 m_requestedIndex;
    };
}
