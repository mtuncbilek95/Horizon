#pragma once

#include <Runtime/Graphics/RHI/Util/DescriptorFormat.h>
#include <Runtime/Graphics/RHI/Util/ShaderStage.h>

#include <vector>

namespace Horizon
{
    struct DescriptorBinding
    {
        u32 binding;
        DescriptorType type;
        u32 count;
        ShaderStage stage;

        DescriptorBinding& setBinding(u32 index) { binding = index; return *this; }
        DescriptorBinding& setType(DescriptorType descriptorType) { type = descriptorType; return *this; }
        DescriptorBinding& setCount(u32 descriptorCount) { count = descriptorCount; return *this; }
        DescriptorBinding& setStage(ShaderStage shaderStage) { stage = shaderStage; return *this; }
    };

    struct GfxDescriptorLayoutDesc
    {
        std::vector<DescriptorBinding> bindings;
        DescriptorLayoutFlags flags;

        GfxDescriptorLayoutDesc& addBinding(const DescriptorBinding& binding) { bindings.push_back(binding); return *this; }
        GfxDescriptorLayoutDesc& setFlags(DescriptorLayoutFlags f) { flags = f; return *this; }
    };
}
