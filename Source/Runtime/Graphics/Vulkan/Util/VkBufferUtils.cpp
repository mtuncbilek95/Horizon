#include "VkBufferUtils.h"

#include <magic_enum/magic_enum.hpp>

namespace Horizon
{
	VkBufferUsageFlags VkBufferUtils::GetVkBufferUsage(BufferUsage usage)
	{
		VkBufferUsageFlags flags = 0;

		if (HasFlag(usage, BufferUsage::TransferSrc))
			flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		if (HasFlag(usage, BufferUsage::TransferDst))
			flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		if (HasFlag(usage, BufferUsage::UniformTexel))
			flags |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
		if (HasFlag(usage, BufferUsage::StorageTexel))
			flags |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
		if (HasFlag(usage, BufferUsage::Uniform))
			flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		if (HasFlag(usage, BufferUsage::Storage))
			flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		if (HasFlag(usage, BufferUsage::Index))
			flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		if (HasFlag(usage, BufferUsage::Vertex))
			flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		if (HasFlag(usage, BufferUsage::Indirect))
			flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
		if (HasFlag(usage, BufferUsage::VideoDecodeSrc))
			flags |= VK_BUFFER_USAGE_VIDEO_DECODE_SRC_BIT_KHR;
		if (HasFlag(usage, BufferUsage::VideoDecodeDst))
			flags |= VK_BUFFER_USAGE_VIDEO_DECODE_DST_BIT_KHR;
		if (HasFlag(usage, BufferUsage::ShaderDeviceAddress))
			flags |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
		if (HasFlag(usage, BufferUsage::ResourceDescriptor))
			flags |= VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;
		if (HasFlag(usage, BufferUsage::SamplerDescriptor))
			flags |= VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;

		return flags;
	}

	VkBufferUsageFlags VkBufferUtils::GetVkDescBufferUsage(DescriptorBufferType type)
	{
		switch (type)
		{
		case DescriptorBufferType::Resource:
			return VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;
		case DescriptorBufferType::Sampler:
			return VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;
		default:
			return 0;
		}
	}

	VkBufferUsageFlags2KHR VkBufferUtils::GetVkDescBufferUsage2(DescriptorBufferType type)
	{
		switch (type)
		{
		case DescriptorBufferType::Resource:
			return VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_2_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;
		case DescriptorBufferType::Sampler:
			return VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_2_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;
		default:
			return 0;
		}
	}
}
