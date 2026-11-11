#pragma once

#include <Runtime/RHI/Pipeline/VertexInputRate.h>
#include <Runtime/RHI/Pipeline/PipelineType.h>
#include <Runtime/RHI/Texture/TextureAspect.h>
#include <Runtime/RHI/Texture/TextureFormat.h>
#include <Runtime/RHI/Descriptor/DescriptorType.h>
#include <Runtime/RHI/Pipeline/ColorTarget.h>
#include <Runtime/RHI/Pipeline/DepthStencilState.h>
#include <Runtime/RHI/Pipeline/PrimitiveTopology.h>
#include <Runtime/RHI/Pipeline/RasterizerState.h>
#include <Runtime/RHI/Buffer/IndexType.h>
#include <Runtime/RHI/Command/LoadOp.h>
#include <Runtime/RHI/Command/ResourceState.h>
#include <Runtime/RHI/Command/StoreOp.h>
#include <Runtime/RHI/Queue/QueueType.h>
#include <Runtime/RHI/Sampler/SamplerDesc.h>
#include <Runtime/Containers/Vector/ReadOnlyList.h>

#include <volk/volk.h>

namespace Horizon::RHI
{
	class TextureView;

	struct Helpers
	{
		static VkFormat ToFormat(TextureFormat format);
		static VkImageAspectFlags ToAspectMask(TextureFormat format);
		static b8 IsDepthFormat(TextureFormat format);
		static b8 IsStencilFormat(TextureFormat format);

		static VkDescriptorType ToDescriptorType(DescriptorType type);
		static VkImageLayout ToDescriptorTypeImageLayout(DescriptorType type);
		static VkBlendFactor ToBlendFactor(BlendFactor factor);
		static VkBlendOp ToBlendOp(BlendOp op);
		static VkCompareOp ToCompareOp(CompareOp op);
		static VkStencilOp ToStencilOp(StencilOp op);
		static VkPolygonMode ToPolygonMode(FillMode mode);
		static VkCullModeFlagBits ToCullMode(CullMode mode);
		static VkPrimitiveTopology ToPrimitiveTopology(PrimitiveTopology topology);
		static VkFrontFace ToFrontFace(FrontFace face);
		static VkVertexInputRate ToVertexInputRate(VertexInputRate rate);

		static VkQueueFlagBits ToQueueFlagBit(QueueType type);
		static VkIndexType ToIndexType(IndexType type);

		static VkPipelineStageFlags2 ToPipelineStageFlags(ResourceState state);
		static VkAccessFlags2 ToAccessFlags(ResourceState state);
		static VkImageLayout ToImageLayout(ResourceState state);

		static VkPipelineBindPoint ToPipelineBindPoint(PipelineType type);

		static VkAttachmentLoadOp ToAttachmentLoadOp(LoadOp op);
		static VkAttachmentStoreOp ToAttachmentStoreOp(StoreOp op);

		static VkBorderColor ToBorderColor(SamplerBorderColor color);
		static VkSamplerAddressMode ToAddressMode(SamplerAddressMode mode);
		static VkSamplerCreateInfo ToSamplerCreateInfo(const SamplerDesc& desc);
	};
}