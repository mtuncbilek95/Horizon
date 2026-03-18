#pragma once

#include <Runtime/Graphics/RHI/Object/GfxObject.h>
#include <Runtime/Graphics/RHI/Sync/GfxFenceDesc.h>

namespace Horizon
{
    class GfxFence : public GfxObject
    {
    public:
        GfxFence(const GfxFenceDesc& desc, GfxDevice* pDevice);
        virtual ~GfxFence() override = default;

        virtual void* Fence() const = 0;
        virtual void WaitIdle() const = 0;
        virtual void Reset() const = 0;

    private:
        GfxFenceDesc m_desc;
    };
}
