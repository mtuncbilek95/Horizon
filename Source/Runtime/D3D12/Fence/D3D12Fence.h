#pragma once

#include <Runtime/RHI/Fence/Fence.h>

#include <dxgi1_6.h>
#include <d3d12.h>

namespace Horizon::RHI
{
	class D3D12Fence final : public Fence
	{
		friend class D3D12Device;
	public:
		~D3D12Fence() final;

		u64 GetCompletedValue() final { return m_fence->GetCompletedValue(); }
		void WaitCPU(u64 value) final;

		void SetDebugName(const c8* pName) final;

		ID3D12Fence* Handle() const { return m_fence; }

	private:
		u64 Advance() { return ++m_value; }

	private:
		ID3D12Fence* m_fence = nullptr;
	};
}
