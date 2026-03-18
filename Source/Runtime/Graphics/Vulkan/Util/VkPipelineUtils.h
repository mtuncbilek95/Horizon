#pragma once

#include <Runtime/Graphics/RHI/Util/ImageFormat.h>
#include <Runtime/Graphics/RHI/Util/PipelineFormat.h>
#include <Runtime/Graphics/RHI/Util/PipelineStageFlags.h>
#include <Runtime/Graphics/RHI/Util/ShaderStage.h>

#include <Runtime/Graphics/Vulkan/VkIncludes.h>

namespace Horizon
{
    struct VkPipelineUtils
    {
        static VkCompareOp GetVkCompareOp(CompareOp op);
        static VkPipelineBindPoint GetVkBindPoint(PipelineType type);
        static VkImageLayout GetVkImgLayout(ImageLayout layout);
        static VkAttachmentStoreOp GetVkStoreOp(AttachmentStore op);
        static VkAttachmentLoadOp GetVkLoadOp(AttachmentLoad op);
        static VkShaderStageFlags GetShaderType(ShaderStage stage);
        static VkShaderStageFlagBits GetShaderTypeBit(ShaderStage stage);
        static VkPrimitiveTopology GetVkTopo(PrimitiveTopology topo);
        static VkPolygonMode GetVkPolygonMode(PolygonMode mode);
        static VkCullModeFlags GetVkCullMode(CullMode mode);
        static VkFrontFace GetVkFrontFace(FaceOrientation frontFace);
        static VkBlendFactor GetVkBlendFactor(BlendFactor factor);
        static VkBlendOp GetVkBlendOp(BlendOp op);
        static VkColorComponentFlags GetVkColorComponents(ColorComponent components);
        static VkVertexInputRate GetVkVertexInputRate(VertexInputRate rate);
        static VkLogicOp GetVkLogicOp(LogicOp op);
        static VkDynamicState GetVkDynamicState(DynamicState state);
        static VkPipelineCreateFlags GetVkPipelineFlags(PipelineFlags flags);
        static VkPipelineStageFlags GetVkStageFlags(PipelineStageFlags flags);
    };
}
