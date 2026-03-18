#pragma once

#include <Runtime/Graphics/RHI/Buffer/GfxBufferDesc.h>
#include <Runtime/Graphics/RHI/Object/GfxObject.h>

namespace Horizon
{
    /**
     * @class GfxBuffer
     * @brief General purpose buffer for RHI. It can be used for Vertex, 
     * Index, Uniform(Constant), Storage and Staging. The Descriptor Buffer
     * will be found in Source/Runtime/Graphics/RHI/Descriptor/GfxDescriptorBuffer.h
     */
    class GfxBuffer : public GfxObject
    {
    public:
        GfxBuffer(const GfxBufferDesc& desc, GfxDevice* pDevice);
        virtual ~GfxBuffer() override = default;

        virtual usize DeviceAddress() const = 0;
        virtual void* Buffer() const = 0;
        virtual void* MappedData() const = 0;
        virtual usize MappedSize() const = 0;

        virtual void Update(const ReadArray<u8>& data) = 0;
        virtual void Update(void* buffer, usize size) = 0;
        virtual void Map(const ReadArray<u8>& data) = 0;
        virtual void Map(void* buffer, usize size) = 0;
        virtual void Unmap() = 0;

        usize SizeInBytes() const { return m_desc.sizeInBytes; }

    private:
        GfxBufferDesc m_desc;
    };
}
