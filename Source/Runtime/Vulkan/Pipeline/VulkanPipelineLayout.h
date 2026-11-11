#pragma once

#include <Runtime/RHI/Pipeline/PipelineLayout.h>

#include <span>
#include <volk/volk.h>

namespace Horizon::RHI
{
	class VulkanPipelineLayout final : public PipelineLayout
	{
		friend class VulkanDevice;
	public:
		static constexpr u32 kPushDataOffset = 256 - sizeof(u32);

		VkShaderDescriptorSetAndBindingMappingInfoEXT GetMappingInfo() const {
			return VkShaderDescriptorSetAndBindingMappingInfoEXT { 
				.sType = VK_STRUCTURE_TYPE_SHADER_DESCRIPTOR_SET_AND_BINDING_MAPPING_INFO_EXT, 
				.mappingCount = (u32)m_mappings.GetCount(),
				.pMappings = m_mappings.GetData(), 
			};
		}

	private:
		List<VkDescriptorSetAndBindingMappingEXT> m_mappings;
	};
}