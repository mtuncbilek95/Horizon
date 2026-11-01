#include "D3D12Fence.h"

#include <Runtime/Containers/StringHelper.h>
#include <Runtime/Log/Terminal.h>

#include <cstring>

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

		HRESULT hr = m_fence->SetEventOnCompletion(value, nullptr);
		if (FAILED(hr))
			Terminal::Error(StringHelper::GetTypeName(this), "SetEventOnCompletion failed, value: {}", value);
	}

	void D3D12Fence::SetDebugName(const c8* pName)
	{
		if (!m_fence || !pName)
			return;

		m_fence->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(std::strlen(pName)), pName);
	}
}
