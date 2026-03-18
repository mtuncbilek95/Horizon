#pragma once

namespace Horizon
{
    enum class DescriptorBufferType
    {
        Resource,
        Sampler
    };

    enum class DescriptorType
    {
        Sampler = 0,
        CombinedSamplerImage = 1,
        SampledImage = 2,
        StorageImage = 3,
        UniformTexelBuffer = 4,
        StorageTexelBuffer= 5,
        Uniform = 6,
        Storage = 7,
        DynamicUniform = 8,
        DynamicStorage = 9,
        InputAttachment = 10,
    };

    enum class DescriptorLayoutFlags
    {
        UpdateAfterBind = 1 << 0,
        Push = 1 << 1,
        Buffer = 1 << 2,
        ImmutableSamplers = 1 << 3,
        BindableIndirect = 1 << 4
    };

    enum class DescriptorPoolFlags
    {
        FreeSets = 1 << 0,
        UpdateAfterBind = 1 << 1,
        HostOnly = 1 << 2,
    };
}
