#pragma once
#include <Runtime/RHI/Queue/GfxQueue.h>
#include <Runtime/D3D12/D3D12Backend.h>

namespace Horizon
{
	class D3D12Queue final : public GfxQueue
	{
		friend class D3D12Device;
	public:
		~D3D12Queue() final;

		void Submit(GfxCommandList* const* lists, u32 count) final;
		u64 Signal(GfxFence* fence) final;
		void Wait(GfxFence* fence, u64 value) final;

		ID3D12CommandQueue* Handle() const { return m_queue; }

	private:
		ID3D12CommandQueue* m_queue = nullptr;
	};
}