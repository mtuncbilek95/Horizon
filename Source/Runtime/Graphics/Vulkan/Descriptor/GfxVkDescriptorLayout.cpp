#include "GfxVkDescriptorLayout.h"

#include <Runtime/Graphics/Vulkan/Debug/VDebug.h>
#include <Runtime/Graphics/Vulkan/Device/GfxVkDevice.h>
#include <Runtime/Graphics/Vulkan/Util/VkDescriptorUtils.h>
#include <Runtime/Graphics/Vulkan/Util/VkPipelineUtils.h>

#include <vector>

namespace Horizon
{
	GfxVkDescriptorLayout::GfxVkDescriptorLayout(const GfxDescriptorLayoutDesc& desc, GfxDevice* pDevice) : GfxDescriptorLayout(desc, pDevice)
	{
		std::vector<VkDescriptorSetLayoutBinding> bindings(desc.bindings.size());

		for (u32 i = 0; i < desc.bindings.size(); i++)
		{
			bindings[i].binding = desc.bindings[i].binding;
			bindings[i].descriptorType = VkDescriptorUtils::GetVkDescType(desc.bindings[i].type);
			bindings[i].descriptorCount = desc.bindings[i].count;
			bindings[i].stageFlags = VkPipelineUtils::GetShaderType(desc.bindings[i].stage);
			bindings[i].pImmutableSamplers = nullptr;
		}

		VkDescriptorSetLayoutCreateInfo createInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		createInfo.bindingCount = static_cast<u32>(bindings.size());
		createInfo.pBindings = bindings.data();
		createInfo.flags = VkDescriptorUtils::GetVkDescLayoutFlags(desc.flags);

		VDebug::VkAssert(vkCreateDescriptorSetLayout(VkDevice(Root()->Device()), &createInfo, nullptr, &m_layout), "VDescLayout");

		if (HasFlag(desc.flags, DescriptorLayoutFlags::Buffer))
		{
			VkDeviceSize layoutSize = 0;

			vkGetDescriptorSetLayoutSizeEXT(VkDevice(Root()->Device()), m_layout, &layoutSize);
			SetSizeInBytes(layoutSize);

			for (usize i = 0; i < desc.bindings.size(); i++)
			{
				VkDeviceSize offset = 0;
				vkGetDescriptorSetLayoutBindingOffsetEXT(VkDevice(Root()->Device()), m_layout, desc.bindings[i].binding, &offset);

				SetOffsetBindings(desc.bindings[i].binding, offset);
			}
		}
	}

	GfxVkDescriptorLayout::~GfxVkDescriptorLayout()
	{
		if (m_layout != VK_NULL_HANDLE)
		{
			vkDestroyDescriptorSetLayout(VkDevice(Root()->Device()), m_layout, nullptr);
			m_layout = VK_NULL_HANDLE;
		}
	}

	void* GfxVkDescriptorLayout::DescLayout() const { return static_cast<void*>(m_layout); }
}
