#include "D3D12ComputePipeline.h"

#include <cstring>

namespace Horizon::RHI
{
	D3D12ComputePipeline::~D3D12ComputePipeline()
	{
		if (m_pipelineState)
			m_pipelineState->Release();
	}

	void D3D12ComputePipeline::SetDebugName(const c8* pName)
	{
		if (!m_pipelineState || !pName)
			return;

		m_pipelineState->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(std::strlen(pName)), pName);
	}
}
