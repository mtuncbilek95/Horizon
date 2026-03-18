#pragma once

#include <Runtime/Graphics/RHI/Object/GfxObject.h>
#include <Runtime/Graphics/RHI/Sync/GfxSemaphore.h>

#include <vulkan/vulkan.h>

namespace Horizon
{
    class GfxVkSemaphore final : public GfxSemaphore
    {
    public:
        GfxVkSemaphore(const GfxSemaphoreDesc& desc, GfxDevice* pDevice);
        ~GfxVkSemaphore() override;

        void* Semaphore() const final;
        void WaitIdle() const final;
        void Reset() const final;
        
    private:
        VkSemaphore m_semaphore;
    };
}
