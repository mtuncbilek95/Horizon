#include "D3D12PipelineLayout.h"

#include <cstring>

namespace Horizon::RHI
{
	D3D12PipelineLayout::~D3D12PipelineLayout()
	{
		if (m_rootSignature)
			m_rootSignature->Release();
	}

	void D3D12PipelineLayout::SetDebugName(const c8* pName)
	{
		if (!m_rootSignature || !pName)
			return;

		m_rootSignature->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(std::strlen(pName)), pName);
	}
}