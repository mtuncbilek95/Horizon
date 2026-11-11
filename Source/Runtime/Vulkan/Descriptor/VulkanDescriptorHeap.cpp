#include "VulkanDescriptorHeap.h"

#include <Runtime/Vulkan/Device/VulkanDevice.h>
#include <Runtime/Vulkan/Buffer/VulkanBuffer.h>
#include <Runtime/Vulkan/Sampler/VulkanSampler.h>
#include <Runtime/Vulkan/Texture/VulkanTexture.h>
#include <Runtime/Vulkan/Texture/VulkanTextureView.h>
#include <Runtime/Containers/ScopedLock.h>
#include <Runtime/Containers/StringHelper.h>
#include <Runtime/Log/Terminal.h>

namespace Horizon::RHI
{
	VulkanDescriptorHeap::~VulkanDescriptorHeap()
	{
		if (m_allocation)
		{
			vmaFreeMemory(static_cast<VulkanDevice*>(m_ownerDevice)->GetAllocator(), m_allocation);
			m_allocation = nullptr;
		}
	}

	void VulkanDescriptorHeap::SetDebugName(const char* pName)
	{
		if (!m_heap || !pName)
			return;

        const VkDebugUtilsObjectNameInfoEXT nameInfo {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .objectType = VK_OBJECT_TYPE_BUFFER,
            .objectHandle = (u64)m_heap,
            .pObjectName = pName
        };

        vkSetDebugUtilsObjectNameEXT(static_cast<VulkanDevice *>(m_ownerDevice)->GetDevice(), &nameInfo);
	}

	u32 VulkanDescriptorHeap::Allocate()
	{
		return AllocateRange(1);
	}

	void VulkanDescriptorHeap::Free(u32 index)
	{
		if (index == kInvalid32)
			return;

		FreeRange(index, 1);
	}

	void VulkanDescriptorHeap::Write(u32 index, DescriptorType type, TextureView* pView)
	{
		if (m_type != DescriptorHeapType::Resource || index >= m_capacity)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Texture write is invalid, index: {} capacity: {}", index, m_capacity);
			return;
		}

		const VulkanTextureView* pVkView = static_cast<const VulkanTextureView*>(pView);
		const VulkanTexture* pTexture = static_cast<const VulkanTexture*>(pView->GetTexture());

		VkImageDescriptorInfoEXT imageInfo {
			.sType = VK_STRUCTURE_TYPE_IMAGE_DESCRIPTOR_INFO_EXT,
			.pView = &pVkView->GetCreateInfo(),
		};

		const VkResourceDescriptorDataEXT data {
			.pImage = &imageInfo
		};

		VkResourceDescriptorInfoEXT info {
			.sType = VK_STRUCTURE_TYPE_RESOURCE_DESCRIPTOR_INFO_EXT,
			.data = data
		};

		if (type == DescriptorType::Texture)
		{
			if (!HasFlag(pTexture->GetUsage(), TextureUsage::Sampled))
			{
				Terminal::Error(StringHelper::GetTypeName(this), "Index {} needs a Sampled texture", index);
				return;
			}

			imageInfo.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			info.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		}
		else if (type == DescriptorType::RWTexture)
		{
			if (!HasFlag(pTexture->GetUsage(), TextureUsage::Storage))
			{
				Terminal::Error(StringHelper::GetTypeName(this), "Index {} needs a Storage texture", index);
				return;
			}

			imageInfo.layout = VK_IMAGE_LAYOUT_GENERAL;
			info.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		}
		else
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Index {} is written with a non-texture type", index);
		}

		WriteResourceDescriptor(index, &info);
	}

	void VulkanDescriptorHeap::Write(u32 index, DescriptorType type, Buffer* pBuffer, u64 offset, u64 size, u32 stride)
	{
		if (m_type != DescriptorHeapType::Resource || index >= m_capacity)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Buffer write is invalid, index: {} capacity: {}", index, m_capacity);
			return;
		}

		const VkDeviceAddressRangeEXT addressInfo {
			.address = pBuffer->GetGpuAddress() + offset,
			.size = size
		};

		const VkResourceDescriptorDataEXT data {
			.pAddressRange = &addressInfo
		};

		VkResourceDescriptorInfoEXT info {
			.sType = VK_STRUCTURE_TYPE_RESOURCE_DESCRIPTOR_INFO_EXT,
			.data = data
		};

		if (type == DescriptorType::ConstantBuffer)
		{
			if (!HasFlag(pBuffer->GetUsage(), BufferUsage::Constant))
			{
				Terminal::Error(StringHelper::GetTypeName(this), "Index {} needs a Constant buffer", index);
				return;
			}

			info.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		}
		else if (type == DescriptorType::Buffer || type == DescriptorType::RWBuffer)
		{
			if (!HasFlag(pBuffer->GetUsage(), BufferUsage::Storage))
			{
				Terminal::Error(StringHelper::GetTypeName(this), "Index {} needs a Storage buffer", index);
				return;
			}

			info.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		}
		else
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Index {} is written with a non-buffer type", index);
		}

		WriteResourceDescriptor(index, &info);
	}

	void VulkanDescriptorHeap::Write(u32 index, Sampler* pSampler)
	{
		if (m_type != DescriptorHeapType::Sampler || index >= m_capacity)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Sampler write is invalid, index: {} capacity: {}", index, m_capacity);
			return;
		}

		const VulkanSampler* pVkSampler = static_cast<const VulkanSampler*>(pSampler);

		WriteSamplerDescriptor(index, &pVkSampler->GetCreateInfo());
	}

	u32 VulkanDescriptorHeap::AllocateRange(u32 count)
	{
		ScopedLock lock(m_lock);

		for (usize i = 0; i < m_freeBlocks.GetCount(); i++)
		{
			Block& block = m_freeBlocks[i];
			if (block.count < count)
				continue;

			u32 offset = block.offset;
			block.offset += count;
			block.count -= count;

			if (block.count == 0)
				m_freeBlocks.RemoveAt(i);

			return offset;
		}

		Terminal::Error(StringHelper::GetTypeName(this), "Heap has no room for {} descriptors, capacity: {}", count, m_capacity);
		return kInvalid32;
	}

	void VulkanDescriptorHeap::FreeRange(u32 offset, u32 count)
	{
		if (count == 0)
			return;

		ScopedLock lock(m_lock);

		usize index = 0;
		while (index < m_freeBlocks.GetCount() && m_freeBlocks[index].offset < offset)
			index++;

		m_freeBlocks.PushAt(index, Block{ offset, count });

		if (index + 1 < m_freeBlocks.GetCount())
		{
			const Block& next = m_freeBlocks[index + 1];
			if (offset + count == next.offset)
			{
				m_freeBlocks[index].count += next.count;
				m_freeBlocks.RemoveAt(index + 1);
			}
		}

		if (index > 0)
		{
			Block& prev = m_freeBlocks[index - 1];
			if (prev.offset + prev.count == m_freeBlocks[index].offset)
			{
				prev.count += m_freeBlocks[index].count;
				m_freeBlocks.RemoveAt(index);
			}
		}
	}

	void VulkanDescriptorHeap::WriteSamplerDescriptor(u32 index, const VkSamplerCreateInfo* info) 
	{
		const VkHostAddressRangeEXT hostAddressRange {
			.address = (u8*)m_mappedPtr + index * m_stride,
			.size = m_stride
		};

		vkWriteSamplerDescriptorsEXT(static_cast<VulkanDevice*>(m_ownerDevice)->GetDevice(), 1, 
			static_cast<const VkSamplerCreateInfo *>(info), &hostAddressRange);
	}

	void VulkanDescriptorHeap::WriteResourceDescriptor(u32 index, const VkResourceDescriptorInfoEXT* info) 
	{
		const VkHostAddressRangeEXT hostAddressRange {
			.address = (u8*)m_mappedPtr + index * m_stride,
			.size = m_stride
		};

		vkWriteResourceDescriptorsEXT(static_cast<VulkanDevice*>(m_ownerDevice)->GetDevice(), 1, 
			static_cast<const VkResourceDescriptorInfoEXT *>(info), &hostAddressRange);
	}
}