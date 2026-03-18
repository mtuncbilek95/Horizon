#pragma once

#include <Runtime/Graphics/RHI/Descriptor/GfxDescriptorLayout.h>

#include <Runtime/Graphics/Vulkan/VkIncludes.h>

namespace Horizon
{
    class GfxVkDescriptorLayout : public GfxDescriptorLayout
    {
    public:
        GfxVkDescriptorLayout(const GfxDescriptorLayoutDesc& desc, GfxDevice* pDevice);
        ~GfxVkDescriptorLayout() override;

        void* DescLayout() const final;

    private:
        VkDescriptorSetLayout m_layout;
    };
}
