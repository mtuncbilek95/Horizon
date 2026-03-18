#include "GfxVkFence.h"

#include <Runtime/Graphics/Vulkan/Debug/VDebug.h>
#include <Runtime/Graphics/Vulkan/Device/GfxVkDevice.h>

namespace Horizon
{
    GfxVkFence::GfxVkFence(const GfxFenceDesc& desc, GfxDevice* pDevice) : GfxFence(desc, pDevice)
    {
        VkFenceCreateInfo fenceInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
        fenceInfo.flags = desc.signalled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;
        VDebug::VkAssert(vkCreateFence(VkDevice(Root()->Device()), &fenceInfo, nullptr, &m_fence), "GfxVkFence");
    }

    GfxVkFence::~GfxVkFence()
    {
        if (m_fence != VK_NULL_HANDLE)
        {
            vkDestroyFence(VkDevice(Root()->Device()), m_fence, nullptr);
            m_fence = VK_NULL_HANDLE;
        }
    }

    void GfxVkFence::WaitIdle() const
    {
        VDebug::VkAssert(vkWaitForFences(VkDevice(Root()->Device()), 1, &m_fence, VK_TRUE, u64_max), "GfxVkFence::WaitIdle");
    }

    void GfxVkFence::Reset() const
    {
        VDebug::VkAssert(vkResetFences(VkDevice(Root()->Device()), 1, &m_fence), "VFence");
    }

    void* GfxVkFence::Fence() const { return static_cast<void*>(m_fence); }
}
