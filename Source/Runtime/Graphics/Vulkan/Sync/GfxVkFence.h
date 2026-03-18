#pragma once

#include <Runtime/Graphics/RHI/Object/GfxObject.h>
#include <Runtime/Graphics/RHI/Sync/GfxFence.h>

#include <vulkan/vulkan.h>

namespace Horizon
{
    class GfxVkFence : public GfxFence
    {
    public:
        GfxVkFence(const GfxFenceDesc& desc, GfxDevice* pDevice);
        ~GfxVkFence() override;

        void* Fence() const final;
        void WaitIdle() const final;
        void Reset() const final;

    private:
        VkFence m_fence;
    };
}
