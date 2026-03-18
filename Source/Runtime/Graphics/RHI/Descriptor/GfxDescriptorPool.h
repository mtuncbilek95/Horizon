#pragma once

#include <Runtime/Graphics/RHI/Descriptor/GfxDescriptorPoolDesc.h>
#include <Runtime/Graphics/RHI/Object/GfxObject.h>

namespace Horizon
{
    /**
     * @class GfxDescriptorPool
     * @brief General purpose descriptor pool structure to pool all the shader
     * units that will be defined.
     */
    class GfxDescriptorPool : public GfxObject
    {
    public:
        GfxDescriptorPool(const GfxDescriptorPoolDesc& desc, GfxDevice* pDevice);
        virtual ~GfxDescriptorPool() override = default;

        virtual void* DescPool() const = 0;

    private:
        GfxDescriptorPoolDesc m_desc;
    };
}
