#include "D3D12Texture.h"

#include <cstring>

namespace Horizon::RHI
{
	D3D12Texture::~D3D12Texture()
	{
		if (m_resource)
			m_resource->Release();

		if (m_allocation)
			m_allocation->Release();
	}

	void D3D12Texture::SetDebugName(const c8* pName)
	{
		if (!m_resource || !pName)
			return;

		m_resource->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(std::strlen(pName)), pName);
	}
}