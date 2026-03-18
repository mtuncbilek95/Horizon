#pragma once

#include <Runtime/Graphics/RHI/Descriptor/GfxDescriptorLayoutDesc.h>
#include <Runtime/Graphics/RHI/Object/GfxObject.h>

#include <unordered_map>

namespace Horizon
{
    /**
     * @class GfxDescriptorLayout
     * @brief General purpose layout structure for shader inputs such
     * as textures, cbuffer(uniform), storage etc. It will be used in
     * pipeline classes.
     */
    class GfxDescriptorLayout : public GfxObject
    {
    public:
        GfxDescriptorLayout(const GfxDescriptorLayoutDesc& desc, GfxDevice* pDevice);
        virtual ~GfxDescriptorLayout() override = default;

        virtual void* DescLayout() const = 0;

        usize SizeInBytes() const { return m_sizeInBytes; }
        usize BindingOffset(u32 binding) const { return m_offsetBindings.at(binding); }

    protected:
        void SetSizeInBytes(usize size) { m_sizeInBytes = size; }
        void SetOffsetBindings(u32 binding, usize offset) { m_offsetBindings[binding] = offset; }

    private:
        GfxDescriptorLayoutDesc m_desc;
        std::unordered_map<u32, usize> m_offsetBindings;
        usize m_sizeInBytes;
    };
}
