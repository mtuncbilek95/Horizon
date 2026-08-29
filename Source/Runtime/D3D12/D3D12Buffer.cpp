#include "D3D12Buffer.h"

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>

namespace Horizon::RHI
{
	D3D12Buffer::~D3D12Buffer()
	{
		if (m_mapped)
			Unmap();

		ReleaseViews();

		if (m_resource)
			m_resource->Release();

		if (m_allocation)
			m_allocation->Release();
	}

	void* D3D12Buffer::Map()
	{
		if (m_mapped)
			return m_mapped;

		if (m_desc.memory == GfxMemoryType::GpuOnly)
		{
			Terminal::Error(StringOps::GetName(this), "GpuOnly buffer cannot be mapped");
			return nullptr;
		}

		const D3D12_RANGE readRange = { 0, m_desc.memory == GfxMemoryType::Readback ? m_desc.size : 0 };

		HRESULT hr = m_resource->Map(0, &readRange, &m_mapped);
		CHECK_REASON(hr, "ID3D12Resource - Map");

		if (FAILED(hr))
			return nullptr;

		return m_mapped;
	}

	void D3D12Buffer::Unmap()
	{
		if (!m_mapped)
			return;

		const D3D12_RANGE writeRange = { 0, m_desc.memory == GfxMemoryType::Readback ? 0 : m_desc.size };

		m_resource->Unmap(0, &writeRange);
		m_mapped = nullptr;
	}
}
