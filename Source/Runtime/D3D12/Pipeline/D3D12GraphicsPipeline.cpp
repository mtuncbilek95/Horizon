#include "D3D12GraphicsPipeline.h"

#include <cstring>

namespace Horizon::RHI
{
	D3D12GraphicsPipeline::~D3D12GraphicsPipeline()
	{
		if (m_pipelineState)
			m_pipelineState->Release();
	}

	void D3D12GraphicsPipeline::SetDebugName(const c8* pName)
	{
		if (!m_pipelineState || !pName)
			return;

		m_pipelineState->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(std::strlen(pName)), pName);
	}
}
