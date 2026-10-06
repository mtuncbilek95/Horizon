#include "D3D12QueryHeap.h"

namespace Horizon::RHI
{
	D3D12QueryHeap::~D3D12QueryHeap()
	{
		if (m_heap)
			m_heap->Release();
	}

	void D3D12QueryHeap::SetDebugName(const char* pName)
	{
		Helpers::SetObjectName(m_heap, pName);
	}
}
