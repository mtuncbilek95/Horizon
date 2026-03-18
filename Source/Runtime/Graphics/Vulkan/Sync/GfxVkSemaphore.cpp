#include "GfxVkSemaphore.h"

#include <Runtime/Graphics/Vulkan/Debug/VDebug.h>
#include <Runtime/Graphics/Vulkan/Device/GfxVkDevice.h>

namespace Horizon
{
    GfxVkSemaphore::GfxVkSemaphore(const GfxSemaphoreDesc& desc, GfxDevice* pDevice) : GfxSemaphore(desc, pDevice)
    {
        VkSemaphoreCreateInfo semaphoreInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
        VDebug::VkAssert(vkCreateSemaphore(VkDevice(Root()->Device()), &semaphoreInfo, nullptr, &m_semaphore), "GfxVkSemaphore");
    }

    GfxVkSemaphore::~GfxVkSemaphore()
    {
        if (m_semaphore != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(VkDevice(Root()->Device()), m_semaphore, nullptr);
            m_semaphore = VK_NULL_HANDLE;
        }
    }

    void GfxVkSemaphore::WaitIdle() const 
    {
        VDebug::VkAssert(vkWaitSemaphores(VkDevice(Root()->Device()), nullptr, u64_max), "GfxVkSemaphore::WaitIdle");
    }

    void GfxVkSemaphore::Reset() const 
    {
    }

    void* GfxVkSemaphore::Semaphore() const { return static_cast<void*>(m_semaphore); }
}
