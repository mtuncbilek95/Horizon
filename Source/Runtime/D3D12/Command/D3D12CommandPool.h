#pragma once

#include <Runtime/RHI/Command/CommandPool.h>

#include <dxgi1_6.h>
#include <d3d12.h>

namespace Horizon::RHI
{
	class D3D12CommandPool final : public CommandPool
	{
		friend class D3D12Device;
	public:
		~D3D12CommandPool() final;

		void Reset() final;

		void SetDebugName(const c8* pName) final;

		ID3D12CommandAllocator* Handle() const { return m_allocator; }

	private:
		ID3D12CommandAllocator* m_allocator = nullptr;
	};
}
