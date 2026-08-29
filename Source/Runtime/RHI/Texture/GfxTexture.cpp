#include "GfxTexture.h"

#include <Runtime/RHI/Descriptor/GfxDescriptorHeap.h>

namespace Horizon::RHI
{
	void GfxTexture::ReleaseViews()
	{
		GfxDescriptorSlot* pSlots[] = { &m_shaderView, &m_storageView, &m_renderTargetView, &m_depthStencilView };

		for (GfxDescriptorSlot* pSlot : pSlots)
		{
			if (!pSlot->IsValid())
				continue;

			pSlot->pHeap->Free(pSlot->index);
			*pSlot = {};
		}
	}
}
