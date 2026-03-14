#include "GfxCommandBuffer.h"

namespace Horizon
{
	GfxCommandBuffer::GfxCommandBuffer(const GfxCommandBufferDesc& desc, GfxDevice* pDevice) : GfxObject(pDevice),
		m_desc(desc)
	{

	}

	void GfxCommandBuffer::BindPipeline(GfxPipeline* pipeline) const
	{
		m_boundPipeline = pipeline;
	}
}
