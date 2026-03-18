#pragma once

#include <Runtime/Graphics/RHI/RenderPass/GfxRenderPass.h>

#include <vulkan/vulkan.h>

namespace Horizon
{
    class GfxVkRenderPass final : public GfxRenderPass
    {
    public:
        GfxVkRenderPass(const GfxRenderPassDesc& desc, GfxDevice* pDevice);
        ~GfxVkRenderPass() override;

        void* Pass() const final;

    private:
        VkRenderPass m_pass;
    };
}
