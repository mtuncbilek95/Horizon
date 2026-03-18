#include "GfxSampler.h"

namespace Horizon
{
    GfxSampler::GfxSampler(const GfxSamplerDesc& desc, GfxDevice* pDevice) : GfxObject(pDevice), m_desc(desc)
    {
    }
}
