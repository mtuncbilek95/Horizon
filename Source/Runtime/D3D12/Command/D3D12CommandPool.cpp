#include "D3D12CommandPool.h"

#include <Runtime/Containers/StringHelper.h>
#include <Runtime/Log/Terminal.h>

#include <cstring>

namespace Horizon::RHI
{
	D3D12CommandPool::~D3D12CommandPool()
	{
		if (m_allocator)
			m_allocator->Release();
	}

	void D3D12CommandPool::Reset()
	{
		HRESULT hr = m_allocator->Reset();
		if (FAILED(hr))
			Terminal::Error(StringHelper::GetTypeName(this), "Allocator reset failed, a list may still be recording or in flight");
	}

	void D3D12CommandPool::SetDebugName(const c8* pName)
	{
		if (!m_allocator || !pName)
			return;

		m_allocator->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(std::strlen(pName)), pName);
	}
}
