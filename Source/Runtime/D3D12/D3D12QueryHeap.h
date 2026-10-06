#pragma once

#include <Runtime/RHI/Query/GfxQueryHeap.h>
#include <Runtime/D3D12/D3D12Helpers.h>

namespace Horizon::RHI
{
	class D3D12QueryHeap final : public GfxQueryHeap
	{
		friend class D3D12Device;
	public:
		~D3D12QueryHeap() final;

		void SetDebugName(const char* pName) final;

		ID3D12QueryHeap* Handle() const { return m_heap; }

	private:
		ID3D12QueryHeap* m_heap = nullptr;
	};
}
