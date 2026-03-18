#pragma once

#include <Runtime/Graphics/RHI/Framebuffer/GfxFramebuffer.h>

#include <vulkan/vulkan.h>

#include <vector>

namespace Horizon
{
    class GfxVkFramebuffer final : public GfxFramebuffer
    {
    public:
        GfxVkFramebuffer(const GfxFramebufferDesc& desc, GfxDevice* pDevice);
        ~GfxVkFramebuffer() override;

        void* Framebuffer(usize index = 0) const final;
        void Resize(const Math::Vec3u& size) final;

    private:
        std::vector<VkFramebuffer> m_framebuffers;
    };
}
