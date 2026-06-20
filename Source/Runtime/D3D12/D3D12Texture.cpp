#include "D3D12Texture.h"

#include <Runtime/D3D12/D3D12Device.h>

namespace Horizon
{
	D3D12Texture::~D3D12Texture()
	{
		auto* pDevice = static_cast<D3D12Device*>(m_ownerDevice);

		if (m_shaderView != ~0u) 
			pDevice->FreeDescriptor(GfxDescriptorHeapType::Resource, m_shaderView);

		if (m_accessView != ~0u) 
			pDevice->FreeDescriptor(GfxDescriptorHeapType::Resource, m_accessView);

		if (m_targetViewIndex != ~0u) 
			pDevice->FreeDescriptor(GfxDescriptorHeapType::Color, m_targetViewIndex);

		if (m_depthViewIndex != ~0u) 
			pDevice->FreeDescriptor(GfxDescriptorHeapType::Depth, m_depthViewIndex);

		if (m_allocation)
			m_allocation->Release();

		if (m_resource)
			m_resource->Release();
	}
}