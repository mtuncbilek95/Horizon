#pragma once

#include <Runtime/RHI/Buffer/GfxBufferArena.h>
#include <Runtime/D3D12/D3D12Helpers.h>

namespace Horizon::RHI
{
	class D3D12Device;

	class D3D12BufferArena final : public GfxBufferArena
	{
		friend class D3D12Device;
	public:
		~D3D12BufferArena() final;

		GfxBufferRange Allocate(usize size, usize alignment) final;
		void Free(const GfxBufferRange& range) final;
		void Reset() final;

	private:
		D3D12MA::VirtualBlock* m_block = nullptr;
	};
}
