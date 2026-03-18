#pragma once

#include <Runtime/Graphics/RHI/Descriptor/GfxDescriptorSetDesc.h>
#include <Runtime/Graphics/RHI/Object/GfxObject.h>

namespace Horizon
{
    /**
     * @class GfxDescriptorSet
     * @brief General purpose descriptor set structure send shader unit
     * into the shaders as textures, uniforms(cbuffer), storage or samplers
     * etc.
     */
    class GfxDescriptorSet : public GfxObject
    {
    public:
        GfxDescriptorSet(const GfxDescriptorSetDesc& desc, GfxDevice* pDevice);
        virtual ~GfxDescriptorSet() override = default;

        virtual void* DescSet() const = 0;
        virtual void Update(const GfxSetUpdateDesc& update) const = 0;
        
    protected:
        GfxDescriptorLayout* UsedLayout() const { return m_desc.layout; }
        GfxDescriptorPool* UsedPool() const { return m_desc.pool; }

    private:
        GfxDescriptorSetDesc m_desc;
    };
}
