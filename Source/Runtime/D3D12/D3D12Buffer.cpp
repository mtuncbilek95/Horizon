#include "D3D12Buffer.h"

#include <Runtime/RHI/Descriptor/GfxDescriptorHeap.h>
#include <Runtime/RHI/Device/GfxDevice.h>

namespace Horizon
{
	D3D12Buffer::~D3D12Buffer()
	{
		GfxDescriptorHeap* pResourceHeap = m_ownerDevice->GetDescriptorHeap(GfxDescriptorHeapType::Resource);

		pResourceHeap->Free(m_shaderView);
		pResourceHeap->Free(m_accessView);

		if (m_mapped)
			m_resource->Unmap(0, nullptr);

		if (m_allocation)
			m_allocation->Release();

		if (m_resource)
			m_resource->Release();
	}

	void D3D12Buffer::SetDebugName(const char* pName)
	{
		if (!m_resource)
			return;

		wchar_t wide[128] = {};

		for (u32 i = 0; i < 127 && pName[i] != 0; i++)
			wide[i] = wchar_t(pName[i]);

		m_resource->SetName(wide);
	}
}