#pragma once

#include <Runtime/Graphics/RHI/Image/GfxImageView.h>
#include <Runtime/Graphics/RHI/Image/GfxImageViewDesc.h>

#include <Runtime/Graphics/Vulkan/VkIncludes.h>

namespace Horizon
{
    class GfxVkImageView : public GfxImageView
    {
    public:
        GfxVkImageView(const GfxImageViewDesc& desc, GfxDevice* pDevice);
        ~GfxVkImageView() override;

        void* Image() const final;
        void* View() const final;

    private:
        VkImage m_image;
        VkImageView m_view;
    };
}
