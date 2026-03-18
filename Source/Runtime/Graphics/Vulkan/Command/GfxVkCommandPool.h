#pragma once

#include <Runtime/Graphics/RHI/Command/GfxCommandPool.h>

#include <vulkan/vulkan.h>

#include <memory>

namespace Horizon
{
    class GfxVkCommandPool : public GfxCommandPool
    {
    public:
        GfxVkCommandPool(const GfxCommandPoolDesc& desc, GfxDevice* pDevice);
        ~GfxVkCommandPool() override;

        void* Pool() const override final;
        std::shared_ptr<GfxCommandBuffer> CreateBuffer(CommandLevel lvl) final;

    private:
        VkCommandPool m_pool;
    };
}
