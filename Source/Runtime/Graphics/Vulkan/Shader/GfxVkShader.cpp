#include "GfxVkShader.h"

#include <Runtime/Graphics/Vulkan/Debug/VDebug.h>
#include <Runtime/Graphics/Vulkan/Device/GfxVkDevice.h>

namespace Horizon
{
    GfxVkShader::GfxVkShader(const GfxShaderDesc& desc, GfxDevice* pDevice) : GfxShader(desc, pDevice)
    {
        VkShaderModuleCreateInfo createInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
        createInfo.codeSize = desc.byteCode.size_bytes();
        createInfo.pCode = reinterpret_cast<const u32*>(desc.byteCode.data());

        VDebug::VkAssert(vkCreateShaderModule(VkDevice(Root()->Device()), &createInfo, nullptr, &m_shader), "GfxVkShader");
    }

    GfxVkShader::~GfxVkShader()
    {
        if (m_shader != VK_NULL_HANDLE)
        {
            vkDestroyShaderModule(VkDevice(Root()->Device()), m_shader, nullptr);
            m_shader = VK_NULL_HANDLE;
        }
    }

    void* GfxVkShader::Shader() const { return static_cast<void*>(m_shader); }
}
