#pragma once

#include <Runtime/RHI/Descriptor/GfxDescriptorHeap.h>
#include <Runtime/D3D12/Utils/D3D12Helpers.h>

#include <Runtime/Containers/List.h>

namespace Horizon
{
	class D3D12Device;

	class D3D12DescriptorHeap final : public GfxDescriptorHeap
	{
		friend class D3D12Device;
	public:
		~D3D12DescriptorHeap() final;

		u32 Allocate() final;
		void Free(u32 index) final;

		ID3D12DescriptorHeap* Handle() const { return m_heap; }

		D3D12_CPU_DESCRIPTOR_HANDLE CpuAt(u32 index) const { return { m_cpuStart.ptr + usize(index) * m_descriptorSize }; }
		D3D12_GPU_DESCRIPTOR_HANDLE GpuAt(u32 index) const { return { m_gpuStart.ptr + u64(index) * m_descriptorSize }; }
		u32 IndexOf(D3D12_CPU_DESCRIPTOR_HANDLE handle) const { return u32((handle.ptr - m_cpuStart.ptr) / m_descriptorSize); }

	private:
		ID3D12DescriptorHeap* m_heap = nullptr;
		D3D12_CPU_DESCRIPTOR_HANDLE m_cpuStart{};
		D3D12_GPU_DESCRIPTOR_HANDLE m_gpuStart{};
		u32 m_descriptorSize = 0;
		u32 m_top = 0;
		List<u32> m_freeList;
	};
}