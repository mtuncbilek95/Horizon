#include "GfxVkFramebuffer.h"

#include <Runtime/Graphics/Vulkan/Debug/VDebug.h>
#include <Runtime/Graphics/Vulkan/Device/GfxVkDevice.h>
#include <Runtime/Graphics/Vulkan/Image/GfxVkImageView.h>
#include <Runtime/Graphics/Vulkan/RenderPass/GfxVkRenderPass.h>

#include <vector>

namespace Horizon
{
    GfxVkFramebuffer::GfxVkFramebuffer(const GfxFramebufferDesc& desc, GfxDevice* pDevice) : GfxFramebuffer(desc, pDevice)
    {
        Resize(desc.framebufferSize);
    }

    GfxVkFramebuffer::~GfxVkFramebuffer()
    {
        for (auto& fb : m_framebuffers)
            vkDestroyFramebuffer(VkDevice(Root()->Device()), fb, nullptr);

        m_framebuffers.clear();
    }

    void* GfxVkFramebuffer::Framebuffer(usize index) const
    {
        if (index > m_framebuffers.size())
        {
            Log::Terminal(LogType::Fatal, "GfxVkFrameBuffer::Framebuffer", "Trying to access {} index when there is only {} framebuffers", index, m_framebuffers.size());
            return nullptr;
        }

        return static_cast<void*>(m_framebuffers[index]);
    }

    void GfxVkFramebuffer::Resize(const Math::Vec3u& size)
    {
        for (auto& fb : m_framebuffers)
            vkDestroyFramebuffer(VkDevice(Root()->Device()), fb, nullptr);

        m_framebuffers.clear();

        m_framebuffers.resize(FrameCount());
        for (usize i = 0; i < FrameCount(); ++i)
        {
            std::vector<VkImageView> attachments;

            for (usize imgIndex = 0; imgIndex < ImageCount(i); imgIndex++)
                attachments.push_back(VkImageView(View(i, imgIndex)->View()));

            VkFramebufferCreateInfo fbInfo{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
            fbInfo.renderPass = VkRenderPass(RenderPass()->Pass());
            fbInfo.attachmentCount = static_cast<u32>(attachments.size());
            fbInfo.pAttachments = attachments.data();
            fbInfo.width = size.x;
            fbInfo.height = size.y;
            fbInfo.layers = size.z;

            VDebug::VkAssert(vkCreateFramebuffer(VkDevice(Root()->Device()), &fbInfo, nullptr, &m_framebuffers[i]), "VFramebuffer");
        }
    }
}
