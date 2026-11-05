#include "D3D12TextureView.h"

#include <Runtime/D3D12/Descriptor/D3D12CpuDescriptorPool.h>

namespace Horizon::RHI
{
	D3D12TextureView::~D3D12TextureView()
	{
		if (m_rtvHandle.ptr != 0)
			m_rtvPool->Free(m_rtvHandle);

		if (m_dsvHandle.ptr != 0)
			m_dsvPool->Free(m_dsvHandle);

		if (m_dsvReadOnlyHandle.ptr != 0)
			m_dsvPool->Free(m_dsvReadOnlyHandle);
	}
}