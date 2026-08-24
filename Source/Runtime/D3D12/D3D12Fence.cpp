#include "D3D12Fence.h"

namespace Horizon::RHI
{
	D3D12Fence::~D3D12Fence()
	{
		if (m_fence)
			m_fence->Release();
	}

	void D3D12Fence::WaitCPU(u64 value)
	{
		if (m_fence->GetCompletedValue() >= value)
			return;

		m_fence->SetEventOnCompletion(value, nullptr);
	}
}