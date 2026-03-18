#pragma once

#include <Runtime/Graphics/RHI/Object/GfxObject.h>
#include <Runtime/Graphics/RHI/Pipeline/GfxPipelineDesc.h>

namespace Horizon
{
    class GfxPipeline : public GfxObject
    {
    public:
        GfxPipeline(const GfxGraphicsPipelineDesc& desc, GfxDevice* pDevice);
        GfxPipeline(const GfxComputePipelineDesc& desc, GfxDevice* pDevice);
        virtual ~GfxPipeline() override = default;

        virtual void* Pipeline() const = 0;
        virtual void* PipelineLayout() const = 0;
        virtual void* PipelineCache() const = 0;

        PipelineType Type() const { return m_type; }

    private:
        GfxGraphicsPipelineDesc m_gDesc;
        GfxComputePipelineDesc m_cDesc;

        PipelineType m_type;
    };
}
