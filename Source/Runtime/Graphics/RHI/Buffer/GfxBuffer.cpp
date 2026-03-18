#include "GfxBuffer.h"

namespace Horizon
{
    GfxBuffer::GfxBuffer(const GfxBufferDesc& desc, GfxDevice* pDevice) : GfxObject(pDevice), 
        m_desc(desc)
    {
    }
}
