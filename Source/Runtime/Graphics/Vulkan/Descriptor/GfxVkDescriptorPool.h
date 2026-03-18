#pragma once

#include <Runtime/Graphics/RHI/Descriptor/GfxDescriptorPool.h>

#include <vulkan/vulkan.h>

namespace Horizon
{
    class GfxVkDescriptorPool final : public GfxDescriptorPool
    {
    public:
        GfxVkDescriptorPool(const GfxDescriptorPoolDesc& desc, GfxDevice* pDevice);
        ~GfxVkDescriptorPool() override;

        void* DescPool() const final;

    private:
        VkDescriptorPool m_pool;
    };
}
