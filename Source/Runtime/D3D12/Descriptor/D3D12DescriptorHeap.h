#pragma once

#include <Runtime/Containers/Vector/List.h>
#include <Runtime/PAL/Sync/Mutex.h>
#include <Runtime/RHI/Descriptor/DescriptorHeap.h>

#include <dxgi1_6.h>
#include <d3d12.h>

namespace Horizon::RHI
{
	class D3D12DescriptorHeap final : public DescriptorHeap
	{
		friend class D3D12Device;

		struct Block
		{
			u32 offset = 0;
			u32 count = 0;
		};

	public:
		~D3D12DescriptorHeap() final;

		void SetDebugName(const c8* pName) final;

		u32 Allocate() final;
		void Free(u32 index) final;

		void Write(u32 index, DescriptorType type, TextureView* pView) final;
		void Write(u32 index, DescriptorType type, Buffer* pBuffer, u64 offset, u64 size, u32 stride = 0) final;
		void Write(u32 index, Sampler* pSampler) final;

		u32 AllocateRange(u32 count);
		void FreeRange(u32 offset, u32 count);

		ID3D12DescriptorHeap* Handle() const { return m_heap; }

		D3D12_CPU_DESCRIPTOR_HANDLE CpuAt(u32 index) const { return { m_cpuStart.ptr + static_cast<SIZE_T>(index) * m_increment }; }
		D3D12_GPU_DESCRIPTOR_HANDLE GpuAt(u32 index) const { return { m_gpuStart.ptr + static_cast<u64>(index) * m_increment }; }

		u64 GetGpuHandle(u32 index) const final { return GpuAt(index).ptr; }
		u32 IndexOf(D3D12_CPU_DESCRIPTOR_HANDLE handle) const { return static_cast<u32>((handle.ptr - m_cpuStart.ptr) / m_increment); }

	private:
		ID3D12Device10* m_device = nullptr;
		ID3D12DescriptorHeap* m_heap = nullptr;

		D3D12_CPU_DESCRIPTOR_HANDLE m_cpuStart = {};
		D3D12_GPU_DESCRIPTOR_HANDLE m_gpuStart = {};
		u32 m_increment = 0;

		List<Block> m_freeBlocks;
		PAL::Mutex m_lock;
	};
}