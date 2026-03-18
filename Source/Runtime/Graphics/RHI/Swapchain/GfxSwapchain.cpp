#include "GfxSwapchain.h"

namespace Horizon
{
    GfxSwapchain::GfxSwapchain(const GfxSwapchainDesc& desc, GfxDevice* pDevice) : GfxObject(pDevice),
        m_desc(desc)
    {
    }

    void GfxSwapchain::Resize(const Math::Vec2u& newSize)
    {
        m_views.clear();
        m_views.shrink_to_fit();

        m_images.clear();
        m_images.shrink_to_fit();
    }
}
