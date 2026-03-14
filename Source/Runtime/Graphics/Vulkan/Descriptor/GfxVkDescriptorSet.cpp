#include "GfxVkDescriptorSet.h"

#include <Runtime/Graphics/Vulkan/Buffer/GfxVkBuffer.h>
#include <Runtime/Graphics/Vulkan/Debug/VDebug.h>
#include <Runtime/Graphics/Vulkan/Descriptor/GfxVkDescriptorLayout.h>
#include <Runtime/Graphics/Vulkan/Descriptor/GfxVkDescriptorPool.h>
#include <Runtime/Graphics/Vulkan/Device/GfxVkDevice.h>
#include <Runtime/Graphics/Vulkan/Image/GfxVkImageView.h>
#include <Runtime/Graphics/Vulkan/Sampler/GfxVkSampler.h>
#include <Runtime/Graphics/Vulkan/Util/VkDescriptorUtils.h>

#include <magic_enum/magic_enum.hpp>

namespace Horizon
{
	GfxVkDescriptorSet::GfxVkDescriptorSet(const GfxDescriptorSetDesc& desc, GfxDevice* pDevice) : GfxDescriptorSet(desc, pDevice)
	{
		VkDescriptorSetLayout dLayout = VkDescriptorSetLayout(desc.layout->DescLayout());

		VkDescriptorSetAllocateInfo info = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		info.descriptorSetCount = 1;
		info.descriptorPool = VkDescriptorPool(desc.pool->DescPool());
		info.pSetLayouts = &dLayout;

		VDebug::VkAssert(vkAllocateDescriptorSets(VkDevice(Root()->Device()), &info, &m_set), "GfxVkDescriptorSet");
	}

	GfxVkDescriptorSet::~GfxVkDescriptorSet()
	{
		if (m_set != VK_NULL_HANDLE)
		{
			vkFreeDescriptorSets(VkDevice(Root()->Device()), VkDescriptorPool(UsedPool()->DescPool()), 1, &m_set);
			m_set = VK_NULL_HANDLE;
		}
	}

	void GfxVkDescriptorSet::Update(const GfxSetUpdateDesc& update) const
	{
		VkWriteDescriptorSet writeInformations[32] = {};
		VkDescriptorBufferInfo writeBufferInformations[32] = {};
		VkDescriptorImageInfo writeImageInformations[32] = {};

		u32 bufferIndex = 0;
		u32 imageIndex = 0;

		for (u8 i = 0; i < update.entries.size(); i++)
		{
			VkWriteDescriptorSet writeInfo = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
			writeInfo.dstSet = m_set;
			writeInfo.dstBinding = update.entries[i].binding;
			writeInfo.dstArrayElement = update.entries[i].arrayElement;
			writeInfo.descriptorCount = update.entries[i].count;
			writeInfo.descriptorType = VkDescriptorUtils::GetVkDescType(update.entries[i].type);
			writeInfo.pNext = nullptr;

			switch (update.entries[i].type)
			{
			case DescriptorType::CombinedSamplerImage:
			{
				VkDescriptorImageInfo imageInfo = {};
				imageInfo.imageView   = VkImageView(static_cast<GfxVkImageView*>(update.entries[i].resource)->View());
				imageInfo.sampler     = VkSampler(static_cast<GfxVkSampler*>(update.entries[i].sampler)->Sampler());
				imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				writeImageInformations[imageIndex] = imageInfo;
				writeInfo.pImageInfo     = &writeImageInformations[imageIndex];
				writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				imageIndex++;
				break;
			}
			case DescriptorType::StorageImage:
			{
				VkDescriptorImageInfo imageInfo = {};
				imageInfo.imageView   = VkImageView(static_cast<GfxVkImageView*>(update.entries[i].resource)->View());
				imageInfo.sampler     = VK_NULL_HANDLE;
				imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
				writeImageInformations[imageIndex] = imageInfo;
				writeInfo.pImageInfo     = &writeImageInformations[imageIndex];
				writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
				imageIndex++;
				break;
			}
			case DescriptorType::Uniform:
			{
				VkDescriptorBufferInfo bufferInfo = {};
				bufferInfo.buffer = VkBuffer(static_cast<GfxVkBuffer*>(update.entries[i].resource)->Buffer());
				bufferInfo.offset = update.entries[i].bufferOffset;
				bufferInfo.range = static_cast<GfxVkBuffer*>(update.entries[i].resource)->SizeInBytes();
				writeBufferInformations[bufferIndex] = bufferInfo;
				writeInfo.pBufferInfo    = &writeBufferInformations[bufferIndex];
				writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				bufferIndex++;
				break;
			}
			default:
				Log::Terminal(LogType::Warning, "Render", "{} is not a valid descriptor type.", magic_enum::enum_name<DescriptorType>(update.entries[i].type));
				break;
			}

			writeInformations[i] = writeInfo;
		}

		vkUpdateDescriptorSets(VkDevice(Root()->Device()), static_cast<u32>(update.entries.size()), writeInformations, 0, nullptr);
	}

	void* GfxVkDescriptorSet::DescSet() const { return static_cast<void*>(m_set); }
}
