#pragma once

#include <Runtime/Graphics/RHI/Descriptor/GfxDescriptorBufferDesc.h>
#include <Runtime/Graphics/RHI/Object/GfxObject.h>

namespace Horizon
{
    class GfxDescriptorBuffer : public GfxObject
    {
    public:
        GfxDescriptorBuffer(const GfxDescriptorBufferDesc& desc, GfxDevice* pDevice);
        virtual ~GfxDescriptorBuffer() override = default;

        virtual void RequestPayload(GfxObject* pObject, usize offsetInBytes) = 0;
        virtual u64 DeviceAddress() const = 0;
        virtual void* MappedData() const = 0;
        
        usize SizeInBytes() const { return m_desc.sizeInBytes; }
        DescriptorBufferType Type() const { return m_desc.type; }
        DescriptorType UseType() const { return m_desc.usage; }

    private:
        GfxDescriptorBufferDesc m_desc;
    };
}