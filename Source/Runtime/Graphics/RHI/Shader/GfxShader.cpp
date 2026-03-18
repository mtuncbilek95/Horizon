#include "GfxShader.h"

namespace Horizon
{
    GfxShader::GfxShader(const GfxShaderDesc& desc, GfxDevice* pDevice) : GfxObject(pDevice), 
        m_desc(desc)
    {
    }
}
