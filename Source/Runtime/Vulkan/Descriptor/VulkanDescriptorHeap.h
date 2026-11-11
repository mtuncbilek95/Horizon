#pragma once

#include <Runtime/Containers/Vector/List.h>
#include <Runtime/PAL/Sync/Mutex.h>
#include <Runtime/RHI/Descriptor/DescriptorHeap.h>
#include <Runtime/Vulkan/Buffer/VulkanBuffer.h>

#include <volk/volk.h>

namespace Horizon::RHI
{
	class VulkanDescriptorHeap final : public DescriptorHeap
	{
		friend class VulkanDevice;

		struct Block
		{
			u32 offset = 0;
			u32 count = 0;
		};

	public:
		~VulkanDescriptorHeap() final;

		void SetDebugName(const c8* pName) final;

		u32 Allocate() final;
		void Free(u32 index) final;

		void Write(u32 index, DescriptorType type, TextureView* pView) final;
		void Write(u32 index, DescriptorType type, Buffer* pBuffer, u64 offset, u64 size, u32 stride = 0) final;
		void Write(u32 index, Sampler* pSampler) final;

		u32 AllocateRange(u32 count);
		void FreeRange(u32 offset, u32 count);

		u64 GetGpuHandle(u32 index) const final { return m_gpuAddress + (index * m_stride); }

		VkBuffer Handle() const { return m_heap; }
		VmaAllocation Allocation() const { return m_allocation; }
		u32 GetStride() const { return m_stride; }
		u32 GetReservedForDriver() const { return m_reservedForDriver; }
		u64 GpuAddress() const { return m_gpuAddress; }
		u8* MappedPtr() const { return m_mappedPtr; }
	private:
		void WriteSamplerDescriptor(u32 index, const VkSamplerCreateInfo* info);
		void WriteResourceDescriptor(u32 index, const VkResourceDescriptorInfoEXT* info);

		VkBuffer m_heap = VK_NULL_HANDLE;
		VmaAllocation m_allocation = nullptr;
		u8* m_mappedPtr = nullptr;
		u64 m_gpuAddress = 0;
		u32 m_stride = 0;
		u32 m_reservedForDriver = 0;

		List<Block> m_freeBlocks;
		PAL::Mutex m_lock;
	};
}