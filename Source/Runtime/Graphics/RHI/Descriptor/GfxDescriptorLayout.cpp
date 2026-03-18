#include "GfxDescriptorLayout.h"

namespace Horizon
{
    GfxDescriptorLayout::GfxDescriptorLayout(const GfxDescriptorLayoutDesc& desc, GfxDevice* pDevice) : GfxObject(pDevice),
        m_desc(desc), m_sizeInBytes(0)
    {
    }
}
