#pragma once

#include <Runtime/Graphics/RHI/Object/GfxObject.h>
#include <Runtime/Graphics/RHI/Sync/GfxSemaphoreDesc.h>

namespace Horizon
{
    class GfxSemaphore : public GfxObject
    {
    public:
        GfxSemaphore(const GfxSemaphoreDesc& desc, GfxDevice* pDevice);
        virtual ~GfxSemaphore() override = default;

        virtual void* Semaphore() const = 0;
        virtual void WaitIdle() const = 0;
        virtual void Reset() const = 0;

    private:
        GfxSemaphoreDesc m_desc;
    };
}
