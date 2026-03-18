#pragma once

#include <Runtime/Graphics/RHI/Pipeline/GfxPipeline.h>

#include <vulkan/vulkan.h>

namespace Horizon
{
    class GfxVkPipeline : public GfxPipeline
    {
    public:
        GfxVkPipeline(const GfxGraphicsPipelineDesc& desc, GfxDevice* pDevice);
        GfxVkPipeline(const GfxComputePipelineDesc& desc, GfxDevice* pDevice);
        ~GfxVkPipeline() override;

        void* Pipeline() const final;
        void* PipelineLayout() const final;
        void* PipelineCache() const final;

    private:
        VkPipeline m_pipeline;
        VkPipelineLayout m_layout;
        VkPipelineCache m_cache;
    };
}
