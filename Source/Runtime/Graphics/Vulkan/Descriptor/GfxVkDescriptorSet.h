#pragma once

#include <Runtime/Graphics/RHI/Descriptor/GfxDescriptorSet.h>

#include <Runtime/Graphics/Vulkan/VkIncludes.h>

namespace Horizon
{
    class GfxVkDescriptorSet : public GfxDescriptorSet
    {
    public:
        GfxVkDescriptorSet(const GfxDescriptorSetDesc& desc, GfxDevice* pDevice);
        ~GfxVkDescriptorSet() override;

        void* DescSet() const final;
        void Update(const GfxSetUpdateDesc& update) const final;

    private:
        VkDescriptorSet m_set;
    };
}
