#pragma once

#include <Runtime/Graphics/RHI/Descriptor/GfxDescriptorBuffer.h>

#include <vk_mem_alloc.h>

namespace Horizon
{
    class GfxVkDescriptorBuffer final : public GfxDescriptorBuffer
    {
    public:
        GfxVkDescriptorBuffer(const GfxDescriptorBufferDesc& desc, GfxDevice* pDevice);
        ~GfxVkDescriptorBuffer() override;

        void RequestPayload(GfxObject* pObject, usize offsetInBytes) final;
        u64 DeviceAddress() const final;
        void* MappedData() const final;

    private:
        VkBuffer m_buffer;
        VmaAllocation m_allocation;
        VmaAllocationInfo m_allocationInfo;
        u64 m_deviceAddress;
    };
}