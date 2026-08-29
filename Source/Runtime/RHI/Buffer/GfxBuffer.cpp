#include "GfxBuffer.h"

#include <Runtime/RHI/Descriptor/GfxDescriptorHeap.h>

namespace Horizon::RHI
{
	void GfxBuffer::ReleaseViews()
	{
		GfxDescriptorSlot* pSlots[] = { &m_shaderView, &m_storageView };

		for (GfxDescriptorSlot* pSlot : pSlots)
		{
			if (!pSlot->IsValid())
				continue;

			pSlot->pHeap->Free(pSlot->index);
			*pSlot = {};
		}
	}
}
