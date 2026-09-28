#include "D3D12Texture.h"

namespace Horizon::RHI
{
	D3D12Texture::~D3D12Texture()
	{
		if (m_resource)
			m_resource->Release();

		if (m_allocation)
			m_allocation->Release();
	}

	void D3D12Texture::SetDebugName(const char* pName)
	{
		Helpers::SetObjectName(m_resource, pName);
	}
}
