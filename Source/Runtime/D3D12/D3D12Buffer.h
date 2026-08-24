#pragma once

#include <Runtime/RHI/Buffer/GfxBuffer.h>
#include <Runtime/D3D12/D3D12Helpers.h>

namespace Horizon::RHI
{
	class D3D12Device;
	class D3D12DescriptorHeap;

	class D3D12Buffer final : public GfxBuffer
	{
		friend class D3D12Device;
		friend class D3D12DescriptorHeap;
		friend class D3D12BufferArena;
		friend class D3D12UploadRing;
	public:
		~D3D12Buffer() final;

		void* Map() final;
		void Unmap() final;

		ID3D12Resource* Handle() const { return m_resource; }

	private:
		ID3D12Resource* m_resource = nullptr;
		D3D12MA::Allocation* m_allocation = nullptr;
		void* m_mapped = nullptr;
	};
}
