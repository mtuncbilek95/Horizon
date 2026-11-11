#include "VulkanHelpers.h"
#include "vulkan/vulkan_core.h"

#include <Runtime/RHI/Texture/Texture.h>
#include <Runtime/RHI/Texture/TextureView.h>

namespace Horizon::RHI
{
	VkFormat Helpers::ToFormat(TextureFormat format)
	{
		switch (format)
		{
		case TextureFormat::Undefined: return VK_FORMAT_UNDEFINED;

		case TextureFormat::R8_UNORM:  return VK_FORMAT_R8_UNORM;
		case TextureFormat::R8_SNORM:  return VK_FORMAT_R8_SNORM;
		case TextureFormat::R8_UINT:   return VK_FORMAT_R8_UINT;
		case TextureFormat::R8_SINT:   return VK_FORMAT_R8_SINT;

		case TextureFormat::RG8_UNORM: return VK_FORMAT_R8G8_UNORM;
		case TextureFormat::RG8_SNORM: return VK_FORMAT_R8G8_SNORM;
		case TextureFormat::RG8_UINT:  return VK_FORMAT_R8G8_UINT;
		case TextureFormat::RG8_SINT:  return VK_FORMAT_R8G8_SINT;

		case TextureFormat::RGBA8_UNORM: return VK_FORMAT_R8G8B8A8_UNORM;
		case TextureFormat::RGBA8_SRGB:  return VK_FORMAT_R8G8B8A8_SRGB;
		case TextureFormat::RGBA8_SNORM: return VK_FORMAT_R8G8B8A8_SNORM;
		case TextureFormat::RGBA8_UINT:  return VK_FORMAT_R8G8B8A8_UINT;
		case TextureFormat::RGBA8_SINT:  return VK_FORMAT_R8G8B8A8_SINT;

		case TextureFormat::BGRA8_UNORM: return VK_FORMAT_B8G8R8A8_UNORM;
		case TextureFormat::BGRA8_SRGB:  return VK_FORMAT_B8G8R8A8_SRGB;

		case TextureFormat::R16_UNORM: return VK_FORMAT_R16_UNORM;
		case TextureFormat::R16_SNORM: return VK_FORMAT_R16_SNORM;
		case TextureFormat::R16_UINT:  return VK_FORMAT_R16_UINT;
		case TextureFormat::R16_SINT:  return VK_FORMAT_R16_SINT;
		case TextureFormat::R16_FLOAT: return VK_FORMAT_R16_SFLOAT;

		case TextureFormat::RG16_UNORM: return VK_FORMAT_R16G16_UNORM;
		case TextureFormat::RG16_SNORM: return VK_FORMAT_R16G16_SNORM;
		case TextureFormat::RG16_UINT:  return VK_FORMAT_R16G16_UINT;
		case TextureFormat::RG16_SINT:  return VK_FORMAT_R16G16_SINT;
		case TextureFormat::RG16_FLOAT: return VK_FORMAT_R16G16_SFLOAT;

		case TextureFormat::RGBA16_UNORM: return VK_FORMAT_R16G16B16A16_UNORM;
		case TextureFormat::RGBA16_SNORM: return VK_FORMAT_R16G16B16A16_SNORM;
		case TextureFormat::RGBA16_UINT:  return VK_FORMAT_R16G16B16A16_UINT;
		case TextureFormat::RGBA16_SINT:  return VK_FORMAT_R16G16B16A16_SINT;
		case TextureFormat::RGBA16_FLOAT: return VK_FORMAT_R16G16B16A16_SFLOAT;

		case TextureFormat::R32_UINT:  return VK_FORMAT_R32_UINT;
		case TextureFormat::R32_SINT:  return VK_FORMAT_R32_SINT;
		case TextureFormat::R32_FLOAT: return VK_FORMAT_R32_SFLOAT;

		case TextureFormat::RG32_UINT:  return VK_FORMAT_R32G32_UINT;
		case TextureFormat::RG32_SINT:  return VK_FORMAT_R32G32_SINT;
		case TextureFormat::RG32_FLOAT: return VK_FORMAT_R32G32_SFLOAT;

		case TextureFormat::RGB32_UINT:  return VK_FORMAT_R32G32B32_UINT;
		case TextureFormat::RGB32_SINT:  return VK_FORMAT_R32G32B32_SINT;
		case TextureFormat::RGB32_FLOAT: return VK_FORMAT_R32G32B32_SFLOAT;

		case TextureFormat::RGBA32_UINT:  return VK_FORMAT_R32G32B32A32_UINT;
		case TextureFormat::RGBA32_SINT:  return VK_FORMAT_R32G32B32A32_SINT;
		case TextureFormat::RGBA32_FLOAT: return VK_FORMAT_R32G32B32A32_SFLOAT;

		case TextureFormat::RGB10A2_UNORM:   return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
		case TextureFormat::RGB10A2_UINT:    return VK_FORMAT_A2B10G10R10_UINT_PACK32;
		case TextureFormat::R11G11B10_FLOAT: return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
		case TextureFormat::RGB9E5_FLOAT:    return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;

		case TextureFormat::D16_UNORM:           return VK_FORMAT_D16_UNORM;
		case TextureFormat::D32_FLOAT:           return VK_FORMAT_D32_SFLOAT;
		case TextureFormat::D24_UNORM_S8_UINT:   return VK_FORMAT_D24_UNORM_S8_UINT;
		case TextureFormat::D32_FLOAT_S8_UINT:   return VK_FORMAT_D32_SFLOAT_S8_UINT;

		case TextureFormat::BC1_UNORM: return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
		case TextureFormat::BC1_SRGB:  return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
		case TextureFormat::BC2_UNORM: return VK_FORMAT_BC2_UNORM_BLOCK;
		case TextureFormat::BC2_SRGB:  return VK_FORMAT_BC2_SRGB_BLOCK;
		case TextureFormat::BC3_UNORM: return VK_FORMAT_BC3_UNORM_BLOCK;
		case TextureFormat::BC3_SRGB:  return VK_FORMAT_BC3_SRGB_BLOCK;
		case TextureFormat::BC4_UNORM: return VK_FORMAT_BC4_UNORM_BLOCK;
		case TextureFormat::BC4_SNORM: return VK_FORMAT_BC4_SNORM_BLOCK;
		case TextureFormat::BC5_UNORM: return VK_FORMAT_BC5_UNORM_BLOCK;
		case TextureFormat::BC5_SNORM: return VK_FORMAT_BC5_SNORM_BLOCK;
		case TextureFormat::BC6H_UFLOAT: return VK_FORMAT_BC6H_UFLOAT_BLOCK;
		case TextureFormat::BC6H_SFLOAT: return VK_FORMAT_BC6H_SFLOAT_BLOCK;
		case TextureFormat::BC7_UNORM: return VK_FORMAT_BC7_UNORM_BLOCK;
		case TextureFormat::BC7_SRGB:  return VK_FORMAT_BC7_SRGB_BLOCK;

		default: return VK_FORMAT_UNDEFINED;
		}
	}

	b8 Helpers::IsStencilFormat(TextureFormat format)
	{
		switch (format)
		{
		case TextureFormat::D24_UNORM_S8_UINT:
		case TextureFormat::D32_FLOAT_S8_UINT:
			return true;
		default:
			return false;
		}
	}

	VkImageAspectFlags Helpers::ToAspectMask(TextureFormat format)
	{
		switch (format)
		{
		case TextureFormat::D16_UNORM:
		case TextureFormat::D32_FLOAT:
			return VK_IMAGE_ASPECT_DEPTH_BIT;
		case TextureFormat::D24_UNORM_S8_UINT:
		case TextureFormat::D32_FLOAT_S8_UINT:
			return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		default:
			return VK_IMAGE_ASPECT_COLOR_BIT;
		}
	}

	VkImageLayout Helpers::ToDescriptorTypeImageLayout(DescriptorType type)
	{
		switch (type)
		{
		case DescriptorType::Texture: return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		case DescriptorType::RWTexture: return VK_IMAGE_LAYOUT_GENERAL;
		default: return VK_IMAGE_LAYOUT_UNDEFINED;
		}
	}

	VkDescriptorType Helpers::ToDescriptorType(DescriptorType type)
	{
		switch (type)
		{
		case DescriptorType::Texture: return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		case DescriptorType::RWTexture: return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		case DescriptorType::ConstantBuffer: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		case DescriptorType::Buffer: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		case DescriptorType::RWBuffer: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		case DescriptorType::Sampler: return VK_DESCRIPTOR_TYPE_SAMPLER;
		default: return VK_DESCRIPTOR_TYPE_MAX_ENUM;
		}
	}

	VkBlendFactor Helpers::ToBlendFactor(BlendFactor factor)
	{
		switch (factor)
		{
		case BlendFactor::Zero: return VK_BLEND_FACTOR_ZERO;
		case BlendFactor::One: return VK_BLEND_FACTOR_ONE;
		case BlendFactor::SrcColor: return VK_BLEND_FACTOR_SRC_COLOR;
		case BlendFactor::InvSrcColor: return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
		case BlendFactor::SrcAlpha: return VK_BLEND_FACTOR_SRC_ALPHA;
		case BlendFactor::InvSrcAlpha: return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		case BlendFactor::DstColor: return VK_BLEND_FACTOR_DST_COLOR;
		case BlendFactor::InvDstColor: return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
		case BlendFactor::DstAlpha: return VK_BLEND_FACTOR_DST_ALPHA;
		case BlendFactor::InvDstAlpha: return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
		case BlendFactor::SrcAlphaSaturate: return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
		case BlendFactor::ConstantColor: return VK_BLEND_FACTOR_CONSTANT_COLOR;
		case BlendFactor::InvConstantColor: return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
		default: return VK_BLEND_FACTOR_MAX_ENUM;
		}
	}

	VkBlendOp Helpers::ToBlendOp(BlendOp op)
	{
		
		switch (op)
		{
		case BlendOp::Add: return VK_BLEND_OP_ADD;
		case BlendOp::Subtract: return VK_BLEND_OP_SUBTRACT;
		case BlendOp::ReverseSubtract: return VK_BLEND_OP_REVERSE_SUBTRACT;
		case BlendOp::Min: return VK_BLEND_OP_MIN;
		case BlendOp::Max: return VK_BLEND_OP_MAX;
		default: return VK_BLEND_OP_MAX_ENUM;
		}
	}

	VkCompareOp Helpers::ToCompareOp(CompareOp op)
	{
		switch (op)
		{
		case CompareOp::Never: return VK_COMPARE_OP_NEVER;
		case CompareOp::Less: return VK_COMPARE_OP_LESS;
		case CompareOp::Equal: return VK_COMPARE_OP_EQUAL;
		case CompareOp::LessEqual: return VK_COMPARE_OP_LESS_OR_EQUAL;
		case CompareOp::Greater: return VK_COMPARE_OP_GREATER;
		case CompareOp::NotEqual: return VK_COMPARE_OP_NOT_EQUAL;
		case CompareOp::GreaterEqual: return VK_COMPARE_OP_GREATER_OR_EQUAL;
		case CompareOp::Always: return VK_COMPARE_OP_ALWAYS;
		default: return VK_COMPARE_OP_MAX_ENUM;
		}
	}

	VkStencilOp Helpers::ToStencilOp(StencilOp op)
	{
		switch (op)
		{
		case StencilOp::Keep: return VK_STENCIL_OP_KEEP;
		case StencilOp::Zero: return VK_STENCIL_OP_ZERO;
		case StencilOp::Replace: return VK_STENCIL_OP_REPLACE;
		case StencilOp::IncrSat: return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
		case StencilOp::DecrSat: return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
		case StencilOp::Invert: return VK_STENCIL_OP_INVERT;
		case StencilOp::IncrWrap: return VK_STENCIL_OP_INCREMENT_AND_WRAP;
		case StencilOp::DecrWrap: return VK_STENCIL_OP_DECREMENT_AND_WRAP;
		default: return VK_STENCIL_OP_KEEP;
		}
	}

	VkPolygonMode Helpers::ToPolygonMode(FillMode mode)
	{
		switch (mode)
		{
		case FillMode::Solid: return VK_POLYGON_MODE_FILL;
		case FillMode::Wireframe: return VK_POLYGON_MODE_LINE;
		default: return VK_POLYGON_MODE_MAX_ENUM;
		}
	}

	VkCullModeFlagBits Helpers::ToCullMode(CullMode mode)
	{
		switch (mode)
		{
		case CullMode::None: return VK_CULL_MODE_NONE;
		case CullMode::Front: return VK_CULL_MODE_FRONT_BIT;
		case CullMode::Back: return VK_CULL_MODE_BACK_BIT;
		default: return VK_CULL_MODE_FLAG_BITS_MAX_ENUM;
		}
	}

	VkPrimitiveTopology Helpers::ToPrimitiveTopology(PrimitiveTopology topology)
	{
		switch (topology)
		{
		case PrimitiveTopology::PointList: return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		case PrimitiveTopology::LineList: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		case PrimitiveTopology::LineStrip: return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
		case PrimitiveTopology::TriangleList: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		case PrimitiveTopology::TriangleStrip: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		default: return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
		}
	}

	VkFrontFace Helpers::ToFrontFace(FrontFace face)
	{
		switch (face)
		{
		case FrontFace::CW: return VK_FRONT_FACE_CLOCKWISE;
		case FrontFace::CCW: return VK_FRONT_FACE_COUNTER_CLOCKWISE;
		default: return VK_FRONT_FACE_MAX_ENUM;
		}
	}

	VkVertexInputRate Helpers::ToVertexInputRate(VertexInputRate rate)
	{
		switch (rate)
		{
		case VertexInputRate::PerVertex: return VK_VERTEX_INPUT_RATE_VERTEX;
		case VertexInputRate::PerInstance: return VK_VERTEX_INPUT_RATE_INSTANCE;
		default: return VK_VERTEX_INPUT_RATE_MAX_ENUM;
		}
	}

	VkQueueFlagBits Helpers::ToQueueFlagBit(QueueType type)
	{
		switch (type)
		{
		case QueueType::Graphics: return VK_QUEUE_GRAPHICS_BIT;
		case QueueType::Compute: return VK_QUEUE_COMPUTE_BIT;
		case QueueType::Transfer: return VK_QUEUE_TRANSFER_BIT;
		default: return VK_QUEUE_FLAG_BITS_MAX_ENUM;
		}
	}

	VkIndexType Helpers::ToIndexType(IndexType type)
	{
		switch (type)
		{
		case IndexType::Index16: return VK_INDEX_TYPE_UINT16;
		case IndexType::Index32: return VK_INDEX_TYPE_UINT32;
		default: return VK_INDEX_TYPE_UINT32;
		}
	}

	VkPipelineStageFlags2 Helpers::ToPipelineStageFlags(ResourceState state)
	{
		if (state == ResourceState::Undefined || state == ResourceState::Present)
			return VK_PIPELINE_STAGE_2_NONE;

		VkPipelineStageFlags2 sync = VK_PIPELINE_STAGE_2_NONE;

		if (HasFlag(state, ResourceState::Common))
			sync |= VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

		if (HasFlag(state, ResourceState::VertexBuffer))
			sync |= VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT;

		if (HasFlag(state, ResourceState::IndexBuffer))
			sync |= VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT;

		if (HasFlag(state, ResourceState::ConstantBuffer | ResourceState::ShaderResource | ResourceState::UnorderedAccess))
			sync |= VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;

		if (HasFlag(state, ResourceState::IndirectArgument))
			sync |= VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;

		if (HasFlag(state, ResourceState::RenderTarget))
			sync |= VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

		if (HasFlag(state, ResourceState::DepthWrite | ResourceState::DepthRead))
			sync |= VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT_KHR | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT_KHR;

		if (HasFlag(state, ResourceState::CopySrc | ResourceState::CopyDst))
			sync |= VK_PIPELINE_STAGE_2_TRANSFER_BIT;

		return sync;
	}

	VkAccessFlags2 Helpers::ToAccessFlags(ResourceState state)
	{
		if (state == ResourceState::Undefined || state == ResourceState::Present)
			return VK_ACCESS_2_NONE;

		VkAccessFlags2 access = VK_ACCESS_2_NONE;

		if (HasFlag(state, ResourceState::VertexBuffer))
			access |= VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT;

		if (HasFlag(state, ResourceState::IndexBuffer))
			access |= VK_ACCESS_2_INDEX_READ_BIT;

		if (HasFlag(state, ResourceState::ConstantBuffer))
			access |= VK_ACCESS_2_UNIFORM_READ_BIT;

		if (HasFlag(state, ResourceState::IndirectArgument))
			access |= VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;

		if (HasFlag(state, ResourceState::ShaderResource))
			access |= VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;

		if (HasFlag(state, ResourceState::UnorderedAccess))
			access |= VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT | VK_ACCESS_2_SHADER_STORAGE_READ_BIT;

		if (HasFlag(state, ResourceState::RenderTarget))
			access |= VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;

		if (HasFlag(state, ResourceState::DepthWrite))
			access |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		if (HasFlag(state, ResourceState::DepthRead))
			access |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

		if (HasFlag(state, ResourceState::CopySrc))
			access |= VK_ACCESS_2_TRANSFER_READ_BIT;

		if (HasFlag(state, ResourceState::CopyDst))
			access |= VK_ACCESS_2_TRANSFER_WRITE_BIT;

		return access;
	}

	VkImageLayout Helpers::ToImageLayout(ResourceState state)
	{
		if (state == ResourceState::Undefined)
			return VK_IMAGE_LAYOUT_UNDEFINED;

		if (HasFlag(state, ResourceState::Present))
			return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		if (HasFlag(state, ResourceState::RenderTarget))
			return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		if (HasFlag(state, ResourceState::DepthWrite))
			return VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL; // depth stencil must be different

		if (HasFlag(state, ResourceState::DepthRead))
			return VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL; // depth stencil must be different

		if (HasFlag(state, ResourceState::UnorderedAccess))
			return VK_IMAGE_LAYOUT_GENERAL;

		if (HasFlag(state, ResourceState::CopyDst))
			return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

		if (HasFlag(state, ResourceState::CopySrc))
			return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

		if (HasFlag(state, ResourceState::ShaderResource))
			return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		return VK_IMAGE_LAYOUT_GENERAL;
	}

	VkPipelineBindPoint Helpers::ToPipelineBindPoint(PipelineType type)
	{
		switch (type)
		{
		case PipelineType::Graphics: return VK_PIPELINE_BIND_POINT_GRAPHICS;
		case PipelineType::Compute: return VK_PIPELINE_BIND_POINT_COMPUTE;
		default: return VK_PIPELINE_BIND_POINT_MAX_ENUM;
		}
	}

	VkAttachmentLoadOp Helpers::ToAttachmentLoadOp(LoadOp op)
	{
		switch (op)
		{
		case LoadOp::Load: return VK_ATTACHMENT_LOAD_OP_LOAD;
		case LoadOp::Clear: return VK_ATTACHMENT_LOAD_OP_CLEAR;
		case LoadOp::DontCare: return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		default: return VK_ATTACHMENT_LOAD_OP_MAX_ENUM;
		}
	}

	VkAttachmentStoreOp Helpers::ToAttachmentStoreOp(StoreOp op)
	{
		switch (op)
		{
		case StoreOp::Store: return VK_ATTACHMENT_STORE_OP_STORE;
		case StoreOp::DontCare: return VK_ATTACHMENT_STORE_OP_DONT_CARE;
		default: return VK_ATTACHMENT_STORE_OP_MAX_ENUM;
		}
	}

	VkSamplerAddressMode Helpers::ToAddressMode(SamplerAddressMode mode)
	{
		switch (mode)
		{
		case SamplerAddressMode::Repeat: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		case SamplerAddressMode::MirroredRepeat: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		case SamplerAddressMode::ClampToEdge: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		case SamplerAddressMode::ClampToBorder: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		case SamplerAddressMode::MirrorClampToEdge: return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
		default: return VK_SAMPLER_ADDRESS_MODE_MAX_ENUM;
		}
	}

	VkBorderColor Helpers::ToBorderColor(SamplerBorderColor color)
	{
		switch (color)
		{
		case SamplerBorderColor::TransparentBlack: return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
		case SamplerBorderColor::OpaqueBlack: return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
		case SamplerBorderColor::OpaqueWhite: return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		default: return VK_BORDER_COLOR_MAX_ENUM;
		}
	}

	VkSamplerCreateInfo Helpers::ToSamplerCreateInfo(const SamplerDesc& desc)
	{
		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.pNext = nullptr;
		samplerInfo.flags = 0;
		samplerInfo.magFilter = desc.magFilter == SamplerFilter::Linear ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
		samplerInfo.minFilter = desc.minFilter == SamplerFilter::Linear ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
		samplerInfo.mipmapMode = desc.mipFilter == SamplerFilter::Linear ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST;
		samplerInfo.addressModeU = ToAddressMode(desc.addressU);
		samplerInfo.addressModeV = ToAddressMode(desc.addressV);
		samplerInfo.addressModeW = ToAddressMode(desc.addressW);
		samplerInfo.mipLodBias = desc.mipLodBias;
		samplerInfo.anisotropyEnable = desc.maxAnisotropy > 1;
		samplerInfo.maxAnisotropy = desc.maxAnisotropy > 16 ? 16 : desc.maxAnisotropy;
		samplerInfo.compareEnable = desc.compareEnable;
		samplerInfo.compareOp = ToCompareOp(desc.compareOp);
		samplerInfo.minLod = desc.minLod;
		samplerInfo.maxLod = desc.maxLod;
		samplerInfo.borderColor = ToBorderColor(desc.borderColor);

		return samplerInfo;
	}
}