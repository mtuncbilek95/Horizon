#pragma once

#include <Runtime/Graphics/RHI/Shader/GfxShader.h>

#include <Runtime/Graphics/Vulkan/VkIncludes.h>

namespace Horizon
{
    class GfxVkShader final : public GfxShader
    {
    public:
        GfxVkShader(const GfxShaderDesc& desc, GfxDevice* pDevice);
        ~GfxVkShader() override;

        void* Shader() const final;

    private:
        VkShaderModule m_shader;
    };
}
