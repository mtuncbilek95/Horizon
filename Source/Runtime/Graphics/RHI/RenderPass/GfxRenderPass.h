#pragma once

#include <Runtime/Graphics/RHI/Object/GfxObject.h>
#include <Runtime/Graphics/RHI/RenderPass/GfxRenderPassDesc.h>

namespace Horizon
{
    class GfxRenderPass : public GfxObject
    {
    public:
        GfxRenderPass(const GfxRenderPassDesc& desc, GfxDevice* pDevice);
        virtual ~GfxRenderPass() override = default;

        virtual void* Pass() const = 0;

    private:
        GfxRenderPassDesc m_desc;
    };
}
