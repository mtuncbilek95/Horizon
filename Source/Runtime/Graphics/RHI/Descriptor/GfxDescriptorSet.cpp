#include "GfxDescriptorSet.h"

namespace Horizon
{
    GfxDescriptorSet::GfxDescriptorSet(const GfxDescriptorSetDesc& desc, GfxDevice* pDevice) : GfxObject(pDevice),
        m_desc(desc)
    {
    }
}
