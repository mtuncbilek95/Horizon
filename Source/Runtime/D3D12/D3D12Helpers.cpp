#include "D3D12Helpers.h"

namespace Horizon::RHI
{
	D3D12_DESCRIPTOR_HEAP_TYPE Helpers::ToDescriptorHeapType(GfxDescriptorHeapType type)
	{
		switch (type)
		{
		case GfxDescriptorHeapType::Resource:			return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		case GfxDescriptorHeapType::Sampler:			return D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		case GfxDescriptorHeapType::Color:				return D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		case GfxDescriptorHeapType::Depth:				return D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		default:										return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		}
	}

	D3D12_COMMAND_LIST_TYPE Helpers::ToCommandListType(GfxQueueType type)
	{
		switch (type)
		{
		case GfxQueueType::Graphics:					return D3D12_COMMAND_LIST_TYPE_DIRECT;
		case GfxQueueType::Compute:						return D3D12_COMMAND_LIST_TYPE_COMPUTE;
		case GfxQueueType::Transfer:					return D3D12_COMMAND_LIST_TYPE_COPY;
		default:										return D3D12_COMMAND_LIST_TYPE_DIRECT;
		}
	}

	DXGI_FORMAT Helpers::ToFormat(GfxTextureFormat format)
	{
		switch (format)
		{
		case GfxTextureFormat::Undefined:				return DXGI_FORMAT_UNKNOWN;

		case GfxTextureFormat::R8_UNORM:				return DXGI_FORMAT_R8_UNORM;
		case GfxTextureFormat::R8_SNORM:				return DXGI_FORMAT_R8_SNORM;
		case GfxTextureFormat::R8_UINT:					return DXGI_FORMAT_R8_UINT;
		case GfxTextureFormat::R8_SINT:					return DXGI_FORMAT_R8_SINT;

		case GfxTextureFormat::RG8_UNORM:				return DXGI_FORMAT_R8G8_UNORM;
		case GfxTextureFormat::RG8_SNORM:				return DXGI_FORMAT_R8G8_SNORM;
		case GfxTextureFormat::RG8_UINT:				return DXGI_FORMAT_R8G8_UINT;
		case GfxTextureFormat::RG8_SINT:				return DXGI_FORMAT_R8G8_SINT;

		case GfxTextureFormat::RGBA8_UNORM:				return DXGI_FORMAT_R8G8B8A8_UNORM;
		case GfxTextureFormat::RGBA8_SRGB:				return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		case GfxTextureFormat::RGBA8_SNORM:				return DXGI_FORMAT_R8G8B8A8_SNORM;
		case GfxTextureFormat::RGBA8_UINT:				return DXGI_FORMAT_R8G8B8A8_UINT;
		case GfxTextureFormat::RGBA8_SINT:				return DXGI_FORMAT_R8G8B8A8_SINT;

		case GfxTextureFormat::BGRA8_UNORM:				return DXGI_FORMAT_B8G8R8A8_UNORM;
		case GfxTextureFormat::BGRA8_SRGB:				return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;

		case GfxTextureFormat::R16_UNORM:				return DXGI_FORMAT_R16_UNORM;
		case GfxTextureFormat::R16_SNORM:				return DXGI_FORMAT_R16_SNORM;
		case GfxTextureFormat::R16_UINT:				return DXGI_FORMAT_R16_UINT;
		case GfxTextureFormat::R16_SINT:				return DXGI_FORMAT_R16_SINT;
		case GfxTextureFormat::R16_FLOAT:				return DXGI_FORMAT_R16_FLOAT;

		case GfxTextureFormat::RG16_UNORM:				return DXGI_FORMAT_R16G16_UNORM;
		case GfxTextureFormat::RG16_SNORM:				return DXGI_FORMAT_R16G16_SNORM;
		case GfxTextureFormat::RG16_UINT:				return DXGI_FORMAT_R16G16_UINT;
		case GfxTextureFormat::RG16_SINT:				return DXGI_FORMAT_R16G16_SINT;
		case GfxTextureFormat::RG16_FLOAT:				return DXGI_FORMAT_R16G16_FLOAT;

		case GfxTextureFormat::RGBA16_UNORM:			return DXGI_FORMAT_R16G16B16A16_UNORM;
		case GfxTextureFormat::RGBA16_SNORM:			return DXGI_FORMAT_R16G16B16A16_SNORM;
		case GfxTextureFormat::RGBA16_UINT:				return DXGI_FORMAT_R16G16B16A16_UINT;
		case GfxTextureFormat::RGBA16_SINT:				return DXGI_FORMAT_R16G16B16A16_SINT;
		case GfxTextureFormat::RGBA16_FLOAT:			return DXGI_FORMAT_R16G16B16A16_FLOAT;

		case GfxTextureFormat::R32_UINT:				return DXGI_FORMAT_R32_UINT;
		case GfxTextureFormat::R32_SINT:				return DXGI_FORMAT_R32_SINT;
		case GfxTextureFormat::R32_FLOAT:				return DXGI_FORMAT_R32_FLOAT;

		case GfxTextureFormat::RG32_UINT:				return DXGI_FORMAT_R32G32_UINT;
		case GfxTextureFormat::RG32_SINT:				return DXGI_FORMAT_R32G32_SINT;
		case GfxTextureFormat::RG32_FLOAT:				return DXGI_FORMAT_R32G32_FLOAT;

		case GfxTextureFormat::RGB32_UINT:				return DXGI_FORMAT_R32G32B32_UINT;
		case GfxTextureFormat::RGB32_SINT:				return DXGI_FORMAT_R32G32B32_SINT;
		case GfxTextureFormat::RGB32_FLOAT:				return DXGI_FORMAT_R32G32B32_FLOAT;

		case GfxTextureFormat::RGBA32_UINT:				return DXGI_FORMAT_R32G32B32A32_UINT;
		case GfxTextureFormat::RGBA32_SINT:				return DXGI_FORMAT_R32G32B32A32_SINT;
		case GfxTextureFormat::RGBA32_FLOAT:			return DXGI_FORMAT_R32G32B32A32_FLOAT;

		case GfxTextureFormat::RGB10A2_UNORM:			return DXGI_FORMAT_R10G10B10A2_UNORM;
		case GfxTextureFormat::RGB10A2_UINT:			return DXGI_FORMAT_R10G10B10A2_UINT;
		case GfxTextureFormat::R11G11B10_FLOAT:			return DXGI_FORMAT_R11G11B10_FLOAT;
		case GfxTextureFormat::RGB9E5_FLOAT:			return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;

		case GfxTextureFormat::D16_UNORM:				return DXGI_FORMAT_D16_UNORM;
		case GfxTextureFormat::D32_FLOAT:				return DXGI_FORMAT_D32_FLOAT;
		case GfxTextureFormat::D24_UNORM_S8_UINT:		return DXGI_FORMAT_D24_UNORM_S8_UINT;
		case GfxTextureFormat::D32_FLOAT_S8_UINT:		return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

		case GfxTextureFormat::BC1_UNORM:				return DXGI_FORMAT_BC1_UNORM;
		case GfxTextureFormat::BC1_SRGB:				return DXGI_FORMAT_BC1_UNORM_SRGB;
		case GfxTextureFormat::BC2_UNORM:				return DXGI_FORMAT_BC2_UNORM;
		case GfxTextureFormat::BC2_SRGB:				return DXGI_FORMAT_BC2_UNORM_SRGB;
		case GfxTextureFormat::BC3_UNORM:				return DXGI_FORMAT_BC3_UNORM;
		case GfxTextureFormat::BC3_SRGB:				return DXGI_FORMAT_BC3_UNORM_SRGB;
		case GfxTextureFormat::BC4_UNORM:				return DXGI_FORMAT_BC4_UNORM;
		case GfxTextureFormat::BC4_SNORM:				return DXGI_FORMAT_BC4_SNORM;
		case GfxTextureFormat::BC5_UNORM:				return DXGI_FORMAT_BC5_UNORM;
		case GfxTextureFormat::BC5_SNORM:				return DXGI_FORMAT_BC5_SNORM;
		case GfxTextureFormat::BC6H_UFLOAT:				return DXGI_FORMAT_BC6H_UF16;
		case GfxTextureFormat::BC6H_SFLOAT:				return DXGI_FORMAT_BC6H_SF16;
		case GfxTextureFormat::BC7_UNORM:				return DXGI_FORMAT_BC7_UNORM;
		case GfxTextureFormat::BC7_SRGB:				return DXGI_FORMAT_BC7_UNORM_SRGB;

		default:										return DXGI_FORMAT_UNKNOWN;
		}
	}

	DXGI_FORMAT Helpers::ToIndexFormat(GfxIndexType type)
	{
		switch (type)
		{
		case GfxIndexType::Index16:						return DXGI_FORMAT_R16_UINT;
		case GfxIndexType::Index32:						return DXGI_FORMAT_R32_UINT;
		default:										return DXGI_FORMAT_R32_UINT;
		}
	}

	D3D12_HEAP_TYPE Helpers::ToHeapType(GfxMemoryType type)
	{
		switch (type)
		{
		case GfxMemoryType::GpuOnly:					return D3D12_HEAP_TYPE_DEFAULT;
		case GfxMemoryType::Upload:						return D3D12_HEAP_TYPE_UPLOAD;
		case GfxMemoryType::GpuUpload:					return D3D12_HEAP_TYPE_GPU_UPLOAD;
		case GfxMemoryType::Readback:					return D3D12_HEAP_TYPE_READBACK;
		default:										return D3D12_HEAP_TYPE_DEFAULT;
		}
	}

	D3D12_RESOURCE_STATES Helpers::ToResourceState(GfxResourceState state)
	{
		u32 result = D3D12_RESOURCE_STATE_COMMON;

		if (HasFlag(state, GfxResourceState::VertexBuffer))
			result |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;

		if (HasFlag(state, GfxResourceState::ConstantBuffer))
			result |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;

		if (HasFlag(state, GfxResourceState::IndexBuffer))
			result |= D3D12_RESOURCE_STATE_INDEX_BUFFER;

		if (HasFlag(state, GfxResourceState::IndirectArgument))
			result |= D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;

		if (HasFlag(state, GfxResourceState::ShaderResource))
			result |= D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;

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

		return D3D12_RESOURCE_STATES(result);
	}

	D3D12_RESOURCE_DIMENSION Helpers::ToResourceDimension(GfxTextureType type)
	{
		switch (type)
		{
		case GfxTextureType::Tex1D:						return D3D12_RESOURCE_DIMENSION_TEXTURE1D;
		case GfxTextureType::Tex2D:						return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		case GfxTextureType::Tex3D:						return D3D12_RESOURCE_DIMENSION_TEXTURE3D;
		default:										return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		}
	}

	D3D12_RESOURCE_FLAGS Helpers::ToResourceFlags(GfxTextureUsage usage)
	{
		u32 result = D3D12_RESOURCE_FLAG_NONE;

		if (HasFlag(usage, GfxTextureUsage::RenderTarget))
			result |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

		if (HasFlag(usage, GfxTextureUsage::DepthStencil))
			result |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		if (HasFlag(usage, GfxTextureUsage::Storage))
			result |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

		if (HasFlag(usage, GfxTextureUsage::DepthStencil) && !HasFlag(usage, GfxTextureUsage::Sampled))
			result |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;

		return D3D12_RESOURCE_FLAGS(result);
	}

	D3D12_RESOURCE_FLAGS Helpers::ToResourceFlags(GfxBufferUsage usage)
	{
		u32 result = D3D12_RESOURCE_FLAG_NONE;

		if (HasFlag(usage, GfxBufferUsage::Storage))
			result |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

		if (HasFlag(usage, GfxBufferUsage::AccelerationStructure))
			result |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

		return D3D12_RESOURCE_FLAGS(result);
	}

	D3D12_FILL_MODE Helpers::ToFillMode(GfxFillMode mode)
	{
		switch (mode)
		{
		case GfxFillMode::Solid:						return D3D12_FILL_MODE_SOLID;
		case GfxFillMode::Wireframe:					return D3D12_FILL_MODE_WIREFRAME;
		default:										return D3D12_FILL_MODE_SOLID;
		}
	}

	D3D12_CULL_MODE Helpers::ToCullMode(GfxCullMode mode)
	{
		switch (mode)
		{
		case GfxCullMode::None:							return D3D12_CULL_MODE_NONE;
		case GfxCullMode::Front:						return D3D12_CULL_MODE_FRONT;
		case GfxCullMode::Back:							return D3D12_CULL_MODE_BACK;
		default:										return D3D12_CULL_MODE_BACK;
		}
	}

	BOOL Helpers::ToFrontCounterClockwise(GfxFrontFace face)
	{
		return face == GfxFrontFace::CCW ? TRUE : FALSE;
	}

	D3D12_COMPARISON_FUNC Helpers::ToCompareFunc(GfxCompareOp op)
	{
		switch (op)
		{
		case GfxCompareOp::Never:						return D3D12_COMPARISON_FUNC_NEVER;
		case GfxCompareOp::Less:						return D3D12_COMPARISON_FUNC_LESS;
		case GfxCompareOp::Equal:						return D3D12_COMPARISON_FUNC_EQUAL;
		case GfxCompareOp::LessEqual:					return D3D12_COMPARISON_FUNC_LESS_EQUAL;
		case GfxCompareOp::Greater:						return D3D12_COMPARISON_FUNC_GREATER;
		case GfxCompareOp::NotEqual:					return D3D12_COMPARISON_FUNC_NOT_EQUAL;
		case GfxCompareOp::GreaterEqual:				return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
		case GfxCompareOp::Always:						return D3D12_COMPARISON_FUNC_ALWAYS;
		default:										return D3D12_COMPARISON_FUNC_ALWAYS;
		}
	}

	D3D12_STENCIL_OP Helpers::ToStencilOp(GfxStencilOp op)
	{
		switch (op)
		{
		case GfxStencilOp::Keep:						return D3D12_STENCIL_OP_KEEP;
		case GfxStencilOp::Zero:						return D3D12_STENCIL_OP_ZERO;
		case GfxStencilOp::Replace:						return D3D12_STENCIL_OP_REPLACE;
		case GfxStencilOp::IncrSat:						return D3D12_STENCIL_OP_INCR_SAT;
		case GfxStencilOp::DecrSat:						return D3D12_STENCIL_OP_DECR_SAT;
		case GfxStencilOp::Invert:						return D3D12_STENCIL_OP_INVERT;
		case GfxStencilOp::IncrWrap:					return D3D12_STENCIL_OP_INCR;
		case GfxStencilOp::DecrWrap:					return D3D12_STENCIL_OP_DECR;
		default:										return D3D12_STENCIL_OP_KEEP;
		}
	}

	D3D12_BLEND Helpers::ToBlend(GfxBlendFactor factor)
	{
		switch (factor)
		{
		case GfxBlendFactor::Zero:						return D3D12_BLEND_ZERO;
		case GfxBlendFactor::One:						return D3D12_BLEND_ONE;
		case GfxBlendFactor::SrcColor:					return D3D12_BLEND_SRC_COLOR;
		case GfxBlendFactor::InvSrcColor:				return D3D12_BLEND_INV_SRC_COLOR;
		case GfxBlendFactor::SrcAlpha:					return D3D12_BLEND_SRC_ALPHA;
		case GfxBlendFactor::InvSrcAlpha:				return D3D12_BLEND_INV_SRC_ALPHA;
		case GfxBlendFactor::DstColor:					return D3D12_BLEND_DEST_COLOR;
		case GfxBlendFactor::InvDstColor:				return D3D12_BLEND_INV_DEST_COLOR;
		case GfxBlendFactor::DstAlpha:					return D3D12_BLEND_DEST_ALPHA;
		case GfxBlendFactor::InvDstAlpha:				return D3D12_BLEND_INV_DEST_ALPHA;
		case GfxBlendFactor::SrcAlphaSaturate:			return D3D12_BLEND_SRC_ALPHA_SAT;
		case GfxBlendFactor::ConstantColor:				return D3D12_BLEND_BLEND_FACTOR;
		case GfxBlendFactor::InvConstantColor:			return D3D12_BLEND_INV_BLEND_FACTOR;
		default:										return D3D12_BLEND_ONE;
		}
	}

	D3D12_BLEND_OP Helpers::ToBlendOp(GfxBlendOp op)
	{
		switch (op)
		{
		case GfxBlendOp::Add:							return D3D12_BLEND_OP_ADD;
		case GfxBlendOp::Subtract:						return D3D12_BLEND_OP_SUBTRACT;
		case GfxBlendOp::ReverseSubtract:				return D3D12_BLEND_OP_REV_SUBTRACT;
		case GfxBlendOp::Min:							return D3D12_BLEND_OP_MIN;
		case GfxBlendOp::Max:							return D3D12_BLEND_OP_MAX;
		default:										return D3D12_BLEND_OP_ADD;
		}
	}

	u8 Helpers::ToColorWriteMask(GfxColorWrite mask)
	{
		u8 result = 0;

		if (HasFlag(mask, GfxColorWrite::Red))
			result |= D3D12_COLOR_WRITE_ENABLE_RED;

		if (HasFlag(mask, GfxColorWrite::Green))
			result |= D3D12_COLOR_WRITE_ENABLE_GREEN;

		if (HasFlag(mask, GfxColorWrite::Blue))
			result |= D3D12_COLOR_WRITE_ENABLE_BLUE;

		if (HasFlag(mask, GfxColorWrite::Alpha))
			result |= D3D12_COLOR_WRITE_ENABLE_ALPHA;

		return result;
	}

	D3D12_PRIMITIVE_TOPOLOGY_TYPE Helpers::ToTopologyType(GfxPrimitiveTopology topology)
	{
		switch (topology)
		{
		case GfxPrimitiveTopology::PointList:			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
		case GfxPrimitiveTopology::LineList:			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		case GfxPrimitiveTopology::LineStrip:			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		case GfxPrimitiveTopology::TriangleList:		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		case GfxPrimitiveTopology::TriangleStrip:		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		default:										return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		}
	}

	D3D_PRIMITIVE_TOPOLOGY Helpers::ToTopology(GfxPrimitiveTopology topology)
	{
		switch (topology)
		{
		case GfxPrimitiveTopology::PointList:			return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
		case GfxPrimitiveTopology::LineList:			return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
		case GfxPrimitiveTopology::LineStrip:			return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
		case GfxPrimitiveTopology::TriangleList:		return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		case GfxPrimitiveTopology::TriangleStrip:		return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		default:										return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		}
	}

	u32 Helpers::ToSampleCount(GfxSampleCount count)
	{
		return u32(count);
	}

	D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE Helpers::ToBeginAccess(GfxLoadOp op)
	{
		switch (op)
		{
		case GfxLoadOp::Load:							return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE;
		case GfxLoadOp::Clear:							return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
		case GfxLoadOp::DontCare:						return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_DISCARD;
		default:										return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE;
		}
	}

	D3D12_RENDER_PASS_ENDING_ACCESS_TYPE Helpers::ToEndAccess(GfxStoreOp op)
	{
		switch (op)
		{
		case GfxStoreOp::Store:							return D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
		case GfxStoreOp::DontCare:						return D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD;
		default:										return D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
		}
	}

	u32 Helpers::ToSyncInterval(GfxPresentMode mode)
	{
		return mode == GfxPresentMode::Vsync ? 1 : 0;
	}

	u32 Helpers::ToPresentFlags(GfxPresentMode mode)
	{
		return mode == GfxPresentMode::Vsync ? 0 : DXGI_PRESENT_ALLOW_TEARING;
	}

	DXGI_FORMAT Helpers::ToTypelessFormat(DXGI_FORMAT format)
	{
		switch (format)
		{
		case DXGI_FORMAT_D16_UNORM:						return DXGI_FORMAT_R16_TYPELESS;
		case DXGI_FORMAT_D32_FLOAT:						return DXGI_FORMAT_R32_TYPELESS;
		case DXGI_FORMAT_D24_UNORM_S8_UINT:				return DXGI_FORMAT_R24G8_TYPELESS;
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:			return DXGI_FORMAT_R32G8X24_TYPELESS;
		default:										return format;
		}
	}

	DXGI_FORMAT Helpers::ToDepthSRVFormat(DXGI_FORMAT format)
	{
		switch (format)
		{
		case DXGI_FORMAT_D16_UNORM:						return DXGI_FORMAT_R16_UNORM;
		case DXGI_FORMAT_D32_FLOAT:						return DXGI_FORMAT_R32_FLOAT;
		case DXGI_FORMAT_D24_UNORM_S8_UINT:				return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:			return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
		default:										return format;
		}
	}

	DXGI_FORMAT Helpers::ToSwapchainFormat(DXGI_FORMAT format)
	{
		switch (format)
		{
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:			return DXGI_FORMAT_R8G8B8A8_UNORM;
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:			return DXGI_FORMAT_B8G8R8A8_UNORM;
		default:										return format;
		}
	}

	b8 Helpers::IsDepthFormat(DXGI_FORMAT format)
	{
		switch (format)
		{
		case DXGI_FORMAT_D16_UNORM:						return true;
		case DXGI_FORMAT_D32_FLOAT:						return true;
		case DXGI_FORMAT_D24_UNORM_S8_UINT:				return true;
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:			return true;
		default:										return false;
		}
	}
}
