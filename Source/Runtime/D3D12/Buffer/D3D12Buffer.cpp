#include "D3D12Buffer.h"

#include <cstring>

namespace Horizon::RHI
{
	D3D12Buffer::~D3D12Buffer()
	{
		if (m_mappedData)
			m_resource->Unmap(0, nullptr);

		if (m_allocation)
			m_allocation->Release();
	}

	void D3D12Buffer::SetDebugName(const c8* pName)
	{
		if (!m_resource || !pName)
			return;

		m_resource->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(std::strlen(pName)), pName);
	}
}