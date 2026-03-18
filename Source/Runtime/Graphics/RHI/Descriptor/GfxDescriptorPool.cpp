#include "GfxDescriptorPool.h"

namespace Horizon
{
    GfxDescriptorPool::GfxDescriptorPool(const GfxDescriptorPoolDesc& desc, GfxDevice* pDevice) : GfxObject(pDevice),
        m_desc(desc)
    {
    }
}
