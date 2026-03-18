#include "GfxVkSampler.h"

#include <Runtime/Graphics/Vulkan/Debug/VDebug.h>
#include <Runtime/Graphics/Vulkan/Device/GfxVkDevice.h>
#include <Runtime/Graphics/Vulkan/Util/VkPipelineUtils.h>
#include <Runtime/Graphics/Vulkan/Util/VkSamplerUtils.h>

namespace Horizon
{
    GfxVkSampler::GfxVkSampler(const GfxSamplerDesc& desc, GfxDevice* pDevice) : GfxSampler(desc, pDevice)
    {
        VkSamplerCreateInfo info = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
        info.magFilter = VkSamplerUtils::GetVkFilter(desc.magFilter);
        info.minFilter = VkSamplerUtils::GetVkFilter(desc.minFilter);
        info.mipmapMode = VkSamplerUtils::GetVkMipMap(desc.mipmapMode);
        info.addressModeU = VkSamplerUtils::GetVkSamplerAddress(desc.addressModeU);
        info.addressModeV = VkSamplerUtils::GetVkSamplerAddress(desc.addressModeV);
        info.addressModeW = VkSamplerUtils::GetVkSamplerAddress(desc.addressModeW);
        info.mipLodBias = desc.mipLodBias;
        info.anisotropyEnable = desc.anisotropyEnable ? VK_TRUE : VK_FALSE;
        info.maxAnisotropy = desc.maxAnisotropy;
        info.compareEnable = desc.compareEnable ? VK_TRUE : VK_FALSE;
        info.compareOp = VkPipelineUtils::GetVkCompareOp(desc.compareOp);
        info.minLod = desc.minLod;
        info.maxLod = desc.maxLod;
        info.borderColor = VkSamplerUtils::GetVkColor(desc.borderColor);
        info.unnormalizedCoordinates = VK_FALSE;

        VDebug::VkAssert(vkCreateSampler(VkDevice(Root()->Device()), &info, nullptr, &m_sampler), "GfxVkSampler");
    }

    GfxVkSampler::~GfxVkSampler()
    {
        if (m_sampler != VK_NULL_HANDLE)
        {
            vkDestroySampler(VkDevice(Root()->Device()), m_sampler, nullptr);
            m_sampler = VK_NULL_HANDLE;
        }
    }

    void* GfxVkSampler::Sampler() const { return static_cast<void*>(m_sampler); }
}
