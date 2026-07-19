#pragma once

#include <Runtime/RHI/Queue/GfxQueue.h>
#include <Runtime/D3D12/Utils/D3D12Helpers.h>

namespace Horizon
{
	class D3D12Queue final : public GfxQueue
	{
		friend class D3D12Device;
	public:
		~D3D12Queue() final;

		void Submit(GfxCommandList* const* ppLists, u32 count) final;
		u64 Signal(GfxFence* pFence) final;
		void Wait(GfxFence* pFence, u64 value) final;

		ID3D12CommandQueue* Handle() const { return m_queue; }

	private:
		ID3D12CommandQueue* m_queue = nullptr;
	};
}