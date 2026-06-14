#include "D3D12Buffer.h"

#include <Runtime/Graphics/D3D12/D3D12Device.h>
#include <D3D12MemAlloc.h>

namespace Horizon
{
	D3D12Buffer::~D3D12Buffer()
	{
		auto* pDevice = static_cast<D3D12Device*>(m_ownerDevice);

		if (m_shaderView != kInvalid32)
			pDevice->FreeDescriptor(GfxDescriptorHeapType::Resource, m_shaderView);
		if (m_accessView != kInvalid32)
			pDevice->FreeDescriptor(GfxDescriptorHeapType::Resource, m_accessView);

		if (m_mapped)
			m_resource->Unmap(0, nullptr);

		if (m_allocation)
			m_allocation->Release();
		if (m_resource)
			m_resource->Release();
	}
}