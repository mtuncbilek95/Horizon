#include "D3D12Texture.h"

#include <Runtime/RHI/Descriptor/GfxDescriptorHeap.h>
#include <Runtime/RHI/Device/GfxDevice.h>

namespace Horizon
{
	D3D12Texture::~D3D12Texture()
	{
		m_ownerDevice->GetDescriptorHeap(GfxDescriptorHeapType::Resource)->Free(m_shaderView);
		m_ownerDevice->GetDescriptorHeap(GfxDescriptorHeapType::Resource)->Free(m_accessView);
		m_ownerDevice->GetDescriptorHeap(GfxDescriptorHeapType::Color)->Free(m_targetViewIndex);
		m_ownerDevice->GetDescriptorHeap(GfxDescriptorHeapType::Depth)->Free(m_depthViewIndex);

		if (m_allocation)
			m_allocation->Release();

		if (m_resource)
			m_resource->Release();
	}

	void D3D12Texture::SetDebugName(const char* pName)
	{
		if (!m_resource)
			return;

		wchar_t wide[128] = {};

		for (u32 i = 0; i < 127 && pName[i] != 0; i++)
			wide[i] = wchar_t(pName[i]);

		m_resource->SetName(wide);
	}
}