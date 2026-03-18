#include "VkDescriptorUtils.h"

#include <magic_enum/magic_enum.hpp>

namespace Horizon
{

    VkBufferUsageFlags VkDescriptorUtils::GetVkDescBufferUsage(DescriptorBufferType type)
    {
        if (HasFlag(type, DescriptorBufferType::Resource))
            return VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;

        if (HasFlag(type, DescriptorBufferType::Sampler))
            return VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;

        return VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;
    }

    VkDescriptorType VkDescriptorUtils::GetVkDescType(DescriptorType type)
    {
        switch (type)
        {
        case DescriptorType::Sampler:
            return VK_DESCRIPTOR_TYPE_SAMPLER;
        case DescriptorType::CombinedSamplerImage:
            return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        case DescriptorType::SampledImage:
            return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        case DescriptorType::StorageImage:
            return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        case DescriptorType::UniformTexelBuffer:
            return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
        case DescriptorType::StorageTexelBuffer:
            return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
        case DescriptorType::Uniform:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case DescriptorType::Storage:
            return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case DescriptorType::DynamicUniform:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        case DescriptorType::DynamicStorage:
            return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        case DescriptorType::InputAttachment:
            return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        default:
            return VK_DESCRIPTOR_TYPE_MAX_ENUM;
        }
    }

    VkDescriptorPoolCreateFlags VkDescriptorUtils::GetVkDescPoolFlags(DescriptorPoolFlags flags)
    {
        VkDescriptorPoolCreateFlags result = 0;

        for (auto f : magic_enum::enum_values<DescriptorPoolFlags>())
        {
            if (HasFlag(flags, f))
            {
                switch (f)
                {
                case DescriptorPoolFlags::FreeSets:
                    result |= VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
                    break;
                case DescriptorPoolFlags::UpdateAfterBind:
                    result |= VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
                    break;
                case DescriptorPoolFlags::HostOnly:
                    result |= VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_EXT;
                    break;
                default:
                    break;
                }
            }
        }

        return result;
    }

    VkDescriptorSetLayoutCreateFlags VkDescriptorUtils::GetVkDescLayoutFlags(DescriptorLayoutFlags flags)
    {
        VkDescriptorSetLayoutCreateFlags result = 0;

        if (HasFlag(flags, DescriptorLayoutFlags::UpdateAfterBind))
            result |= VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;

        if (HasFlag(flags, DescriptorLayoutFlags::Push))
            result |= VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;

        if (HasFlag(flags, DescriptorLayoutFlags::Buffer))
            result |= VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;

        if (HasFlag(flags, DescriptorLayoutFlags::ImmutableSamplers))
            result |= VK_DESCRIPTOR_SET_LAYOUT_CREATE_EMBEDDED_IMMUTABLE_SAMPLERS_BIT_EXT;

        if (HasFlag(flags, DescriptorLayoutFlags::BindableIndirect))
            result |= VK_DESCRIPTOR_SET_LAYOUT_CREATE_INDIRECT_BINDABLE_BIT_NV;

        return result;
    }
}
