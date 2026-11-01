#pragma once

#include <Runtime/RHI/Queue/Queue.h>

#include <dxgi1_6.h>
#include <d3d12.h>
#include <comdef.h>
#include <D3D12MemAlloc.h>

namespace Horizon::RHI
{
	class D3D12Queue final : public Queue 
	{
		friend class D3D12Device;
	public:
		~D3D12Queue() final;

		void Submit(List<CommandList*> cmdlists) final;
		u64 Signal(Fence* pFence) final;
		void Wait(Fence* pFence, u64 value) final;

		u64 GetTimestampFrequency() final;

		void SetDebugName(const char* pName) final;

		ID3D12CommandQueue* Handle() const { return m_queue; }

	private:
		ID3D12CommandQueue* m_queue = nullptr;
	};
}