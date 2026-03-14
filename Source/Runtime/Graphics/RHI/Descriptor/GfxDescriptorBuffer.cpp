#include "GfxDescriptorBuffer.h"

namespace Horizon
{
	GfxDescriptorBuffer::GfxDescriptorBuffer(const GfxDescriptorBufferDesc& desc, GfxDevice* pDevice) : GfxObject(pDevice),
		m_desc(desc)
	{
	}
}