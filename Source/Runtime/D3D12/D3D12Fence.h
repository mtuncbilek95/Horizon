#pragma once
#include <Runtime/RHI/Fence/GfxFence.h>
#include <Runtime/D3D12/D3D12Backend.h>

namespace Horizon
{
	class D3D12Fence final : public GfxFence
	{
		friend class D3D12Device;
		friend class D3D12Queue;
	public:
		~D3D12Fence();

		u64 GetCompletedValue() final { return m_fence->GetCompletedValue(); }
		void WaitCPU(u64 value) final;

	private:
		u64 Advance() { return ++m_value; }
		ID3D12Fence* Handle() const { return m_fence; }

		ID3D12Fence* m_fence = nullptr;
		u64 m_value = 0;
	};
}