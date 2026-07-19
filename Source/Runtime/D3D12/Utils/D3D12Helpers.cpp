#include "D3D12Helpers.h"

namespace Horizon
{
	DXGI_FORMAT Helpers::ToDXGIFormat(GfxTextureFormat format)
	{
		switch (format)
		{
		case GfxTextureFormat::Undefined:   return DXGI_FORMAT_UNKNOWN;

		case GfxTextureFormat::R8:          return DXGI_FORMAT_R8_UNORM;
		case GfxTextureFormat::RG8:         return DXGI_FORMAT_R8G8_UNORM;
		case GfxTextureFormat::RGBA8:       return DXGI_FORMAT_R8G8B8A8_UNORM;
		case GfxTextureFormat::RGBA8_sRGB:  return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		case GfxTextureFormat::BGRA8:       return DXGI_FORMAT_B8G8R8A8_UNORM;
		case GfxTextureFormat::BGRA8_sRGB:  return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;

		case GfxTextureFormat::R8_SNORM:    return DXGI_FORMAT_R8_SNORM;
		case GfxTextureFormat::RG8_SNORM:   return DXGI_FORMAT_R8G8_SNORM;

		case GfxTextureFormat::R16F:        return DXGI_FORMAT_R16_FLOAT;
		case GfxTextureFormat::RG16F:       return DXGI_FORMAT_R16G16_FLOAT;
		case GfxTextureFormat::RGBA16F:     return DXGI_FORMAT_R16G16B16A16_FLOAT;

		case GfxTextureFormat::R32F:        return DXGI_FORMAT_R32_FLOAT;
		case GfxTextureFormat::RG32F:       return DXGI_FORMAT_R32G32_FLOAT;
		case GfxTextureFormat::RGBA32F:     return DXGI_FORMAT_R32G32B32A32_FLOAT;

		case GfxTextureFormat::R11G11B10F:  return DXGI_FORMAT_R11G11B10_FLOAT;
		case GfxTextureFormat::RGB10A2:     return DXGI_FORMAT_R10G10B10A2_UNORM;
		case GfxTextureFormat::RGB9E5:      return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;

		case GfxTextureFormat::R8U:         return DXGI_FORMAT_R8_UINT;
		case GfxTextureFormat::R16U:        return DXGI_FORMAT_R16_UINT;
		case GfxTextureFormat::R32U:        return DXGI_FORMAT_R32_UINT;
		case GfxTextureFormat::RG32U:       return DXGI_FORMAT_R32G32_UINT;

		case GfxTextureFormat::D16:         return DXGI_FORMAT_D16_UNORM;
		case GfxTextureFormat::D32:         return DXGI_FORMAT_D32_FLOAT;
		case GfxTextureFormat::D24S8:       return DXGI_FORMAT_D24_UNORM_S8_UINT;
		case GfxTextureFormat::D32S8:       return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

		case GfxTextureFormat::BC1:         return DXGI_FORMAT_BC1_UNORM;
		case GfxTextureFormat::BC1_sRGB:    return DXGI_FORMAT_BC1_UNORM_SRGB;
		case GfxTextureFormat::BC3:         return DXGI_FORMAT_BC3_UNORM;
		case GfxTextureFormat::BC3_sRGB:    return DXGI_FORMAT_BC3_UNORM_SRGB;
		case GfxTextureFormat::BC4:         return DXGI_FORMAT_BC4_UNORM;
		case GfxTextureFormat::BC5:         return DXGI_FORMAT_BC5_UNORM;
		case GfxTextureFormat::BC6H:        return DXGI_FORMAT_BC6H_UF16;
		case GfxTextureFormat::BC7:         return DXGI_FORMAT_BC7_UNORM;
		case GfxTextureFormat::BC7_sRGB:    return DXGI_FORMAT_BC7_UNORM_SRGB;
		}
		return DXGI_FORMAT_UNKNOWN;
	}

	DXGI_FORMAT Helpers::ToTypelessFormat(DXGI_FORMAT format)
	{
		switch (format)
		{
		case DXGI_FORMAT_D16_UNORM:            return DXGI_FORMAT_R16_TYPELESS;
		case DXGI_FORMAT_D32_FLOAT:            return DXGI_FORMAT_R32_TYPELESS;
		case DXGI_FORMAT_D24_UNORM_S8_UINT:    return DXGI_FORMAT_R24G8_TYPELESS;
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT: return DXGI_FORMAT_R32G8X24_TYPELESS;
		default:                               return format;
		}
	}

	DXGI_FORMAT Helpers::ToDepthSRVFormat(DXGI_FORMAT format)
	{
		switch (format)
		{
		case DXGI_FORMAT_D16_UNORM:            return DXGI_FORMAT_R16_UNORM;
		case DXGI_FORMAT_D32_FLOAT:            return DXGI_FORMAT_R32_FLOAT;
		case DXGI_FORMAT_D24_UNORM_S8_UINT:    return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT: return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
		default:                               return format;
		}
	}

	DXGI_FORMAT Helpers::ToIndexFormat(GfxIndexType type)
	{
		switch (type)
		{
		case GfxIndexType::Index16: return DXGI_FORMAT_R16_UINT;
		case GfxIndexType::Index32: return DXGI_FORMAT_R32_UINT;
		}
		return DXGI_FORMAT_R32_UINT;
	}

	b8 Helpers::IsDepthFormat(GfxTextureFormat format)
	{
		switch (format)
		{
		case GfxTextureFormat::D16:
		case GfxTextureFormat::D32:
		case GfxTextureFormat::D24S8:
		case GfxTextureFormat::D32S8:
			return true;
		default:
			return false;
		}
	}

	D3D12_BLEND Helpers::ToBlend(GfxBlendFactor factor)
	{
		switch (factor)
		{
		case GfxBlendFactor::Zero:        return D3D12_BLEND_ZERO;
		case GfxBlendFactor::One:         return D3D12_BLEND_ONE;
		case GfxBlendFactor::SrcColor:    return D3D12_BLEND_SRC_COLOR;
		case GfxBlendFactor::InvSrcColor: return D3D12_BLEND_INV_SRC_COLOR;
		case GfxBlendFactor::SrcAlpha:    return D3D12_BLEND_SRC_ALPHA;
		case GfxBlendFactor::InvSrcAlpha: return D3D12_BLEND_INV_SRC_ALPHA;
		case GfxBlendFactor::DstColor:    return D3D12_BLEND_DEST_COLOR;
		case GfxBlendFactor::InvDstColor: return D3D12_BLEND_INV_DEST_COLOR;
		case GfxBlendFactor::DstAlpha:    return D3D12_BLEND_DEST_ALPHA;
		case GfxBlendFactor::InvDstAlpha: return D3D12_BLEND_INV_DEST_ALPHA;
		}
		return D3D12_BLEND_ONE;
	}

	D3D12_BLEND_OP Helpers::ToBlendOp(GfxBlendOp op)
	{
		switch (op)
		{
		case GfxBlendOp::Add:         return D3D12_BLEND_OP_ADD;
		case GfxBlendOp::Subtract:    return D3D12_BLEND_OP_SUBTRACT;
		case GfxBlendOp::RevSubtract: return D3D12_BLEND_OP_REV_SUBTRACT;
		case GfxBlendOp::Min:         return D3D12_BLEND_OP_MIN;
		case GfxBlendOp::Max:         return D3D12_BLEND_OP_MAX;
		}
		return D3D12_BLEND_OP_ADD;
	}

	D3D12_COMPARISON_FUNC Helpers::ToCompare(GfxCompareOp op)
	{
		switch (op)
		{
		case GfxCompareOp::Never:        return D3D12_COMPARISON_FUNC_NEVER;
		case GfxCompareOp::Less:         return D3D12_COMPARISON_FUNC_LESS;
		case GfxCompareOp::Equal:        return D3D12_COMPARISON_FUNC_EQUAL;
		case GfxCompareOp::LessEqual:    return D3D12_COMPARISON_FUNC_LESS_EQUAL;
		case GfxCompareOp::Greater:      return D3D12_COMPARISON_FUNC_GREATER;
		case GfxCompareOp::NotEqual:     return D3D12_COMPARISON_FUNC_NOT_EQUAL;
		case GfxCompareOp::GreaterEqual: return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
		case GfxCompareOp::Always:       return D3D12_COMPARISON_FUNC_ALWAYS;
		}
		return D3D12_COMPARISON_FUNC_ALWAYS;
	}

	D3D12_STENCIL_OP Helpers::ToStencilOp(GfxStencilOp op)
	{
		switch (op)
		{
		case GfxStencilOp::Keep:     return D3D12_STENCIL_OP_KEEP;
		case GfxStencilOp::Zero:     return D3D12_STENCIL_OP_ZERO;
		case GfxStencilOp::Replace:  return D3D12_STENCIL_OP_REPLACE;
		case GfxStencilOp::IncrSat:  return D3D12_STENCIL_OP_INCR_SAT;
		case GfxStencilOp::DecrSat:  return D3D12_STENCIL_OP_DECR_SAT;
		case GfxStencilOp::Invert:   return D3D12_STENCIL_OP_INVERT;
		case GfxStencilOp::IncrWrap: return D3D12_STENCIL_OP_INCR;
		case GfxStencilOp::DecrWrap: return D3D12_STENCIL_OP_DECR;
		}
		return D3D12_STENCIL_OP_KEEP;
	}

	u8 Helpers::ToColorWriteMask(GfxColorWrite mask)
	{
		u8 result = 0;
		if (HasFlag(mask, GfxColorWrite::R)) result |= D3D12_COLOR_WRITE_ENABLE_RED;
		if (HasFlag(mask, GfxColorWrite::G)) result |= D3D12_COLOR_WRITE_ENABLE_GREEN;
		if (HasFlag(mask, GfxColorWrite::B)) result |= D3D12_COLOR_WRITE_ENABLE_BLUE;
		if (HasFlag(mask, GfxColorWrite::A)) result |= D3D12_COLOR_WRITE_ENABLE_ALPHA;
		return result;
	}

	D3D12_FILL_MODE Helpers::ToFillMode(GfxFillMode mode)
	{
		switch (mode)
		{
		case GfxFillMode::Solid:     return D3D12_FILL_MODE_SOLID;
		case GfxFillMode::Wireframe: return D3D12_FILL_MODE_WIREFRAME;
		}
		return D3D12_FILL_MODE_SOLID;
	}

	D3D12_CULL_MODE Helpers::ToCullMode(GfxCullMode mode)
	{
		switch (mode)
		{
		case GfxCullMode::None:  return D3D12_CULL_MODE_NONE;
		case GfxCullMode::Front: return D3D12_CULL_MODE_FRONT;
		case GfxCullMode::Back:  return D3D12_CULL_MODE_BACK;
		}
		return D3D12_CULL_MODE_BACK;
	}

	b8 Helpers::ToFrontCounterClockwise(GfxFrontFace face)
	{
		return face == GfxFrontFace::CCW;
	}

	D3D12_PRIMITIVE_TOPOLOGY_TYPE Helpers::ToTopologyType(GfxPrimitiveTopology topology)
	{
		switch (topology)
		{
		case GfxPrimitiveTopology::PointList:     return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
		case GfxPrimitiveTopology::LineList:      return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		case GfxPrimitiveTopology::TriangleList:  return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		case GfxPrimitiveTopology::TriangleStrip: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		}
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	}

	D3D_PRIMITIVE_TOPOLOGY Helpers::ToTopology(GfxPrimitiveTopology topology)
	{
		switch (topology)
		{
		case GfxPrimitiveTopology::PointList:     return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
		case GfxPrimitiveTopology::LineList:      return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
		case GfxPrimitiveTopology::TriangleList:  return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		case GfxPrimitiveTopology::TriangleStrip: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		}
		return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	}

	D3D12_RESOURCE_STATES Helpers::ToResourceState(GfxResourceState state)
	{
		if (state == GfxResourceState::Undefined || state == GfxResourceState::Common)
			return D3D12_RESOURCE_STATE_COMMON;

		D3D12_RESOURCE_STATES result = D3D12_RESOURCE_STATE_COMMON;

		if (HasFlag(state, GfxResourceState::VertexBuffer) ||
			HasFlag(state, GfxResourceState::ConstantBuffer))
			result |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;

		if (HasFlag(state, GfxResourceState::IndexBuffer))
			result |= D3D12_RESOURCE_STATE_INDEX_BUFFER;

		if (HasFlag(state, GfxResourceState::IndirectArg))
			result |= D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;

		if (HasFlag(state, GfxResourceState::ShaderResource))
			result |= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

		if (HasFlag(state, GfxResourceState::UnorderedAccess))
			result |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

		if (HasFlag(state, GfxResourceState::RenderTarget))
			result |= D3D12_RESOURCE_STATE_RENDER_TARGET;

		if (HasFlag(state, GfxResourceState::DepthWrite))
			result |= D3D12_RESOURCE_STATE_DEPTH_WRITE;

		if (HasFlag(state, GfxResourceState::DepthRead))
			result |= D3D12_RESOURCE_STATE_DEPTH_READ;

		if (HasFlag(state, GfxResourceState::CopySrc))
			result |= D3D12_RESOURCE_STATE_COPY_SOURCE;

		if (HasFlag(state, GfxResourceState::CopyDst))
			result |= D3D12_RESOURCE_STATE_COPY_DEST;

		if (HasFlag(state, GfxResourceState::ResolveSrc))
			result |= D3D12_RESOURCE_STATE_RESOLVE_SOURCE;

		if (HasFlag(state, GfxResourceState::ResolveDst))
			result |= D3D12_RESOURCE_STATE_RESOLVE_DEST;

		if (HasFlag(state, GfxResourceState::AccelerationStructure))
			result |= D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;

		if (HasFlag(state, GfxResourceState::Present))
			result |= D3D12_RESOURCE_STATE_PRESENT;

		return result;
	}

	D3D12_HEAP_TYPE Helpers::ToHeapType(GfxMemoryType type)
	{
		switch (type)
		{
		case GfxMemoryType::GpuOnly:  return D3D12_HEAP_TYPE_DEFAULT;
		case GfxMemoryType::Upload:   return D3D12_HEAP_TYPE_UPLOAD;
		case GfxMemoryType::Readback: return D3D12_HEAP_TYPE_READBACK;
		}
		return D3D12_HEAP_TYPE_DEFAULT;
	}

	D3D12_DESCRIPTOR_HEAP_TYPE Helpers::ToDescriptorHeapType(GfxDescriptorHeapType type)
	{
		switch (type)
		{
		case GfxDescriptorHeapType::Resource: return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		case GfxDescriptorHeapType::Sampler:  return D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		case GfxDescriptorHeapType::Color:    return D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		case GfxDescriptorHeapType::Depth:    return D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		}
		return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	}

	D3D12_RESOURCE_DIMENSION Helpers::ToResourceDimension(GfxTextureType type)
	{
		switch (type)
		{
		case GfxTextureType::Tex1D: return D3D12_RESOURCE_DIMENSION_TEXTURE1D;
		case GfxTextureType::Tex2D: return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		case GfxTextureType::Tex3D: return D3D12_RESOURCE_DIMENSION_TEXTURE3D;
		}
		return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	}

	D3D12_RESOURCE_FLAGS Helpers::ToResourceFlags(GfxTextureUsage usage)
	{
		D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;
		if (HasFlag(usage, GfxTextureUsage::RenderTarget))
			flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		if (HasFlag(usage, GfxTextureUsage::DepthStencil))
			flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		if (HasFlag(usage, GfxTextureUsage::Storage))
			flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

		if (HasFlag(usage, GfxTextureUsage::DepthStencil) && !HasFlag(usage, GfxTextureUsage::Sampled))
			flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;

		return flags;
	}

	D3D12_RESOURCE_FLAGS Helpers::ToResourceFlags(GfxBufferUsage usage)
	{
		D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;
		if (HasFlag(usage, GfxBufferUsage::Storage)) flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		return flags;
	}

	D3D12_FILTER Helpers::ToFilter(GfxFilter minFilter, GfxFilter magFilter, GfxMipFilter mipFilter, b8 anisotropy, b8 comparison)
	{
		if (anisotropy)
		{
			return comparison ? D3D12_FILTER_COMPARISON_ANISOTROPIC : D3D12_FILTER_ANISOTROPIC;
		}

		const D3D12_FILTER_REDUCTION_TYPE reduction =
			comparison ? D3D12_FILTER_REDUCTION_TYPE_COMPARISON : D3D12_FILTER_REDUCTION_TYPE_STANDARD;

		const D3D12_FILTER_TYPE minType = (minFilter == GfxFilter::Linear) ? D3D12_FILTER_TYPE_LINEAR : D3D12_FILTER_TYPE_POINT;
		const D3D12_FILTER_TYPE magType = (magFilter == GfxFilter::Linear) ? D3D12_FILTER_TYPE_LINEAR : D3D12_FILTER_TYPE_POINT;
		const D3D12_FILTER_TYPE mipType = (mipFilter == GfxMipFilter::Linear) ? D3D12_FILTER_TYPE_LINEAR : D3D12_FILTER_TYPE_POINT;

		return D3D12_ENCODE_BASIC_FILTER(minType, magType, mipType, reduction);
	}

	D3D12_TEXTURE_ADDRESS_MODE Helpers::ToAddressMode(GfxAddressMode mode)
	{
		switch (mode)
		{
		case GfxAddressMode::Repeat:       return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		case GfxAddressMode::MirrorRepeat: return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
		case GfxAddressMode::ClampEdge:    return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		case GfxAddressMode::ClampBorder:  return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		}
		return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	}

	D3D12_STATIC_BORDER_COLOR Helpers::ToStaticBorderColor(GfxBorderColor color)
	{
		switch (color)
		{
		case GfxBorderColor::TransparentBlack: return D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		case GfxBorderColor::OpaqueBlack:      return D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
		case GfxBorderColor::OpaqueWhite:      return D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
		}
		return D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	}

	D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE Helpers::ToBeginAccess(GfxLoadOp op)
	{
		switch (op)
		{
		case GfxLoadOp::Load:     return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE;
		case GfxLoadOp::Clear:    return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
		case GfxLoadOp::DontCare: return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_DISCARD;
		}
		return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE;
	}

	D3D12_RENDER_PASS_ENDING_ACCESS_TYPE Helpers::ToEndAccess(GfxStoreOp op)
	{
		switch (op)
		{
		case GfxStoreOp::Store:    return D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
		case GfxStoreOp::DontCare: return D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD;
		}
		return D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
	}

	D3D12_COMMAND_LIST_TYPE Helpers::ToListType(GfxQueueType type)
	{
		switch (type)
		{
		case GfxQueueType::Graphics: return D3D12_COMMAND_LIST_TYPE_DIRECT;
		case GfxQueueType::Compute:  return D3D12_COMMAND_LIST_TYPE_COMPUTE;
		case GfxQueueType::Transfer: return D3D12_COMMAND_LIST_TYPE_COPY;
		case GfxQueueType::Count:    return D3D12_COMMAND_LIST_TYPE_DIRECT;
		}
		return D3D12_COMMAND_LIST_TYPE_DIRECT;
	}

	u32 Helpers::ToSampleCount(GfxSampleCount count)
	{
		return static_cast<u32>(count);
	}
}