#include "D3D12Helpers.h"

#include <Runtime/RHI/Texture/Texture.h>
#include <Runtime/RHI/Texture/TextureView.h>

namespace Horizon::RHI
{
	DXGI_FORMAT Helpers::ToFormat(TextureFormat format)
	{
		switch (format)
		{
		case TextureFormat::Undefined: return DXGI_FORMAT_UNKNOWN;

		case TextureFormat::R8_UNORM: return DXGI_FORMAT_R8_UNORM;
		case TextureFormat::R8_SNORM: return DXGI_FORMAT_R8_SNORM;
		case TextureFormat::R8_UINT: return DXGI_FORMAT_R8_UINT;
		case TextureFormat::R8_SINT: return DXGI_FORMAT_R8_SINT;

		case TextureFormat::RG8_UNORM: return DXGI_FORMAT_R8G8_UNORM;
		case TextureFormat::RG8_SNORM: return DXGI_FORMAT_R8G8_SNORM;
		case TextureFormat::RG8_UINT: return DXGI_FORMAT_R8G8_UINT;
		case TextureFormat::RG8_SINT: return DXGI_FORMAT_R8G8_SINT;

		case TextureFormat::RGBA8_UNORM: return DXGI_FORMAT_R8G8B8A8_UNORM;
		case TextureFormat::RGBA8_SRGB: return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		case TextureFormat::RGBA8_SNORM: return DXGI_FORMAT_R8G8B8A8_SNORM;
		case TextureFormat::RGBA8_UINT: return DXGI_FORMAT_R8G8B8A8_UINT;
		case TextureFormat::RGBA8_SINT: return DXGI_FORMAT_R8G8B8A8_SINT;

		case TextureFormat::BGRA8_UNORM: return DXGI_FORMAT_B8G8R8A8_UNORM;
		case TextureFormat::BGRA8_SRGB: return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;

		case TextureFormat::R16_UNORM: return DXGI_FORMAT_R16_UNORM;
		case TextureFormat::R16_SNORM: return DXGI_FORMAT_R16_SNORM;
		case TextureFormat::R16_UINT: return DXGI_FORMAT_R16_UINT;
		case TextureFormat::R16_SINT: return DXGI_FORMAT_R16_SINT;
		case TextureFormat::R16_FLOAT: return DXGI_FORMAT_R16_FLOAT;

		case TextureFormat::RG16_UNORM: return DXGI_FORMAT_R16G16_UNORM;
		case TextureFormat::RG16_SNORM: return DXGI_FORMAT_R16G16_SNORM;
		case TextureFormat::RG16_UINT: return DXGI_FORMAT_R16G16_UINT;
		case TextureFormat::RG16_SINT: return DXGI_FORMAT_R16G16_SINT;
		case TextureFormat::RG16_FLOAT: return DXGI_FORMAT_R16G16_FLOAT;

		case TextureFormat::RGBA16_UNORM: return DXGI_FORMAT_R16G16B16A16_UNORM;
		case TextureFormat::RGBA16_SNORM: return DXGI_FORMAT_R16G16B16A16_SNORM;
		case TextureFormat::RGBA16_UINT: return DXGI_FORMAT_R16G16B16A16_UINT;
		case TextureFormat::RGBA16_SINT: return DXGI_FORMAT_R16G16B16A16_SINT;
		case TextureFormat::RGBA16_FLOAT: return DXGI_FORMAT_R16G16B16A16_FLOAT;

		case TextureFormat::R32_UINT: return DXGI_FORMAT_R32_UINT;
		case TextureFormat::R32_SINT: return DXGI_FORMAT_R32_SINT;
		case TextureFormat::R32_FLOAT: return DXGI_FORMAT_R32_FLOAT;

		case TextureFormat::RG32_UINT: return DXGI_FORMAT_R32G32_UINT;
		case TextureFormat::RG32_SINT: return DXGI_FORMAT_R32G32_SINT;
		case TextureFormat::RG32_FLOAT: return DXGI_FORMAT_R32G32_FLOAT;

		case TextureFormat::RGB32_UINT: return DXGI_FORMAT_R32G32B32_UINT;
		case TextureFormat::RGB32_SINT: return DXGI_FORMAT_R32G32B32_SINT;
		case TextureFormat::RGB32_FLOAT: return DXGI_FORMAT_R32G32B32_FLOAT;

		case TextureFormat::RGBA32_UINT: return DXGI_FORMAT_R32G32B32A32_UINT;
		case TextureFormat::RGBA32_SINT: return DXGI_FORMAT_R32G32B32A32_SINT;
		case TextureFormat::RGBA32_FLOAT: return DXGI_FORMAT_R32G32B32A32_FLOAT;

		case TextureFormat::RGB10A2_UNORM: return DXGI_FORMAT_R10G10B10A2_UNORM;
		case TextureFormat::RGB10A2_UINT: return DXGI_FORMAT_R10G10B10A2_UINT;
		case TextureFormat::R11G11B10_FLOAT: return DXGI_FORMAT_R11G11B10_FLOAT;
		case TextureFormat::RGB9E5_FLOAT: return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;

		case TextureFormat::D16_UNORM: return DXGI_FORMAT_D16_UNORM;
		case TextureFormat::D32_FLOAT: return DXGI_FORMAT_D32_FLOAT;
		case TextureFormat::D24_UNORM_S8_UINT: return DXGI_FORMAT_D24_UNORM_S8_UINT;
		case TextureFormat::D32_FLOAT_S8_UINT: return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

		case TextureFormat::BC1_UNORM: return DXGI_FORMAT_BC1_UNORM;
		case TextureFormat::BC1_SRGB: return DXGI_FORMAT_BC1_UNORM_SRGB;
		case TextureFormat::BC2_UNORM: return DXGI_FORMAT_BC2_UNORM;
		case TextureFormat::BC2_SRGB: return DXGI_FORMAT_BC2_UNORM_SRGB;
		case TextureFormat::BC3_UNORM: return DXGI_FORMAT_BC3_UNORM;
		case TextureFormat::BC3_SRGB: return DXGI_FORMAT_BC3_UNORM_SRGB;
		case TextureFormat::BC4_UNORM: return DXGI_FORMAT_BC4_UNORM;
		case TextureFormat::BC4_SNORM: return DXGI_FORMAT_BC4_SNORM;
		case TextureFormat::BC5_UNORM: return DXGI_FORMAT_BC5_UNORM;
		case TextureFormat::BC5_SNORM: return DXGI_FORMAT_BC5_SNORM;
		case TextureFormat::BC6H_UFLOAT: return DXGI_FORMAT_BC6H_UF16;
		case TextureFormat::BC6H_SFLOAT: return DXGI_FORMAT_BC6H_SF16;
		case TextureFormat::BC7_UNORM: return DXGI_FORMAT_BC7_UNORM;
		case TextureFormat::BC7_SRGB: return DXGI_FORMAT_BC7_UNORM_SRGB;

		default: return DXGI_FORMAT_UNKNOWN;
		}
	}

	DXGI_FORMAT Helpers::ToTypelessFormat(TextureFormat format)
	{
		switch (format)
		{
		case TextureFormat::D16_UNORM: return DXGI_FORMAT_R16_TYPELESS;
		case TextureFormat::D32_FLOAT: return DXGI_FORMAT_R32_TYPELESS;
		case TextureFormat::D24_UNORM_S8_UINT: return DXGI_FORMAT_R24G8_TYPELESS;
		case TextureFormat::D32_FLOAT_S8_UINT: return DXGI_FORMAT_R32G8X24_TYPELESS;
		default: return ToFormat(format);
		}
	}

	DXGI_FORMAT Helpers::ToShaderViewFormat(TextureFormat format, TextureAspect aspect)
	{
		b8 isStencil = aspect == TextureAspect::Stencil;

		switch (format)
		{
		case TextureFormat::D16_UNORM: return DXGI_FORMAT_R16_UNORM;
		case TextureFormat::D32_FLOAT: return DXGI_FORMAT_R32_FLOAT;
		case TextureFormat::D24_UNORM_S8_UINT: return isStencil ? DXGI_FORMAT_X24_TYPELESS_G8_UINT : DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		case TextureFormat::D32_FLOAT_S8_UINT: return isStencil ? DXGI_FORMAT_X32_TYPELESS_G8X24_UINT : DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
		default: return ToFormat(format);
		}
	}

	b8 Helpers::IsDepthFormat(TextureFormat format)
	{
		switch (format)
		{
		case TextureFormat::D16_UNORM:
		case TextureFormat::D32_FLOAT:
		case TextureFormat::D24_UNORM_S8_UINT:
		case TextureFormat::D32_FLOAT_S8_UINT:
			return true;
		default:
			return false;
		}
	}

	D3D12_SHADER_RESOURCE_VIEW_DESC Helpers::ToShaderResourceViewDesc(const TextureView& view)
	{
		b8 isMultisampled = view.GetTexture()->GetSampleCount() != SampleCount::Count1;
		u32 planeSlice = view.GetAspect() == TextureAspect::Stencil ? 1 : 0;

		D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
		desc.Format = ToShaderViewFormat(view.GetFormat(), view.GetAspect());
		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		switch (view.GetType())
		{
		case TextureViewType::Texture1D:
			desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
			desc.Texture1D.MostDetailedMip = view.GetBaseMip();
			desc.Texture1D.MipLevels = view.GetMipCount();
			break;
		case TextureViewType::Texture1DArray:
			desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
			desc.Texture1DArray.MostDetailedMip = view.GetBaseMip();
			desc.Texture1DArray.MipLevels = view.GetMipCount();
			desc.Texture1DArray.FirstArraySlice = view.GetBaseLayer();
			desc.Texture1DArray.ArraySize = view.GetLayerCount();
			break;
		case TextureViewType::Texture2D:
			if (isMultisampled)
			{
				desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
			}
			else
			{
				desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				desc.Texture2D.MostDetailedMip = view.GetBaseMip();
				desc.Texture2D.MipLevels = view.GetMipCount();
				desc.Texture2D.PlaneSlice = planeSlice;
			}
			break;
		case TextureViewType::Texture2DArray:
			if (isMultisampled)
			{
				desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
				desc.Texture2DMSArray.FirstArraySlice = view.GetBaseLayer();
				desc.Texture2DMSArray.ArraySize = view.GetLayerCount();
			}
			else
			{
				desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
				desc.Texture2DArray.MostDetailedMip = view.GetBaseMip();
				desc.Texture2DArray.MipLevels = view.GetMipCount();
				desc.Texture2DArray.FirstArraySlice = view.GetBaseLayer();
				desc.Texture2DArray.ArraySize = view.GetLayerCount();
				desc.Texture2DArray.PlaneSlice = planeSlice;
			}
			break;
		case TextureViewType::Texture3D:
			desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
			desc.Texture3D.MostDetailedMip = view.GetBaseMip();
			desc.Texture3D.MipLevels = view.GetMipCount();
			break;
		case TextureViewType::TextureCube:
			desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			desc.TextureCube.MostDetailedMip = view.GetBaseMip();
			desc.TextureCube.MipLevels = view.GetMipCount();
			break;
		case TextureViewType::TextureCubeArray:
			desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
			desc.TextureCubeArray.MostDetailedMip = view.GetBaseMip();
			desc.TextureCubeArray.MipLevels = view.GetMipCount();
			desc.TextureCubeArray.First2DArrayFace = view.GetBaseLayer();
			desc.TextureCubeArray.NumCubes = view.GetLayerCount() / 6;
			break;
		default:
			break;
		}

		return desc;
	}

	D3D12_UNORDERED_ACCESS_VIEW_DESC Helpers::ToUnorderedAccessViewDesc(const TextureView& view)
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {};
		desc.Format = ToFormat(view.GetFormat());

		switch (view.GetType())
		{
		case TextureViewType::Texture1D:
			desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
			desc.Texture1D.MipSlice = view.GetBaseMip();
			break;
		case TextureViewType::Texture1DArray:
			desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
			desc.Texture1DArray.MipSlice = view.GetBaseMip();
			desc.Texture1DArray.FirstArraySlice = view.GetBaseLayer();
			desc.Texture1DArray.ArraySize = view.GetLayerCount();
			break;
		case TextureViewType::Texture2D:
			desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
			desc.Texture2D.MipSlice = view.GetBaseMip();
			break;
		case TextureViewType::Texture2DArray:
		case TextureViewType::TextureCube:
		case TextureViewType::TextureCubeArray:
			desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
			desc.Texture2DArray.MipSlice = view.GetBaseMip();
			desc.Texture2DArray.FirstArraySlice = view.GetBaseLayer();
			desc.Texture2DArray.ArraySize = view.GetLayerCount();
			break;
		case TextureViewType::Texture3D:
			desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
			desc.Texture3D.MipSlice = view.GetBaseMip();
			desc.Texture3D.FirstWSlice = 0;
			desc.Texture3D.WSize = kInvalid32;
			break;
		default:
			break;
		}

		return desc;
	}

	D3D12_RENDER_TARGET_VIEW_DESC Helpers::ToRenderTargetViewDesc(const TextureView& view)
	{
		b8 isMultisampled = view.GetTexture()->GetSampleCount() != SampleCount::Count1;

		D3D12_RENDER_TARGET_VIEW_DESC desc = {};
		desc.Format = ToFormat(view.GetFormat());

		switch (view.GetType())
		{
		case TextureViewType::Texture1D:
			desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
			desc.Texture1D.MipSlice = view.GetBaseMip();
			break;
		case TextureViewType::Texture1DArray:
			desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
			desc.Texture1DArray.MipSlice = view.GetBaseMip();
			desc.Texture1DArray.FirstArraySlice = view.GetBaseLayer();
			desc.Texture1DArray.ArraySize = view.GetLayerCount();
			break;
		case TextureViewType::Texture2D:
			if (isMultisampled)
			{
				desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
			}
			else
			{
				desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
				desc.Texture2D.MipSlice = view.GetBaseMip();
			}
			break;
		case TextureViewType::Texture2DArray:
		case TextureViewType::TextureCube:
		case TextureViewType::TextureCubeArray:
			if (isMultisampled)
			{
				desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
				desc.Texture2DMSArray.FirstArraySlice = view.GetBaseLayer();
				desc.Texture2DMSArray.ArraySize = view.GetLayerCount();
			}
			else
			{
				desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
				desc.Texture2DArray.MipSlice = view.GetBaseMip();
				desc.Texture2DArray.FirstArraySlice = view.GetBaseLayer();
				desc.Texture2DArray.ArraySize = view.GetLayerCount();
			}
			break;
		case TextureViewType::Texture3D:
			desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
			desc.Texture3D.MipSlice = view.GetBaseMip();
			desc.Texture3D.FirstWSlice = 0;
			desc.Texture3D.WSize = kInvalid32;
			break;
		default:
			break;
		}

		return desc;
	}

	D3D12_DEPTH_STENCIL_VIEW_DESC Helpers::ToDepthStencilViewDesc(const TextureView& view)
	{
		b8 isMultisampled = view.GetTexture()->GetSampleCount() != SampleCount::Count1;

		D3D12_DEPTH_STENCIL_VIEW_DESC desc = {};
		desc.Format = ToFormat(view.GetFormat());
		desc.Flags = D3D12_DSV_FLAG_NONE;

		switch (view.GetType())
		{
		case TextureViewType::Texture1D:
			desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
			desc.Texture1D.MipSlice = view.GetBaseMip();
			break;
		case TextureViewType::Texture1DArray:
			desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
			desc.Texture1DArray.MipSlice = view.GetBaseMip();
			desc.Texture1DArray.FirstArraySlice = view.GetBaseLayer();
			desc.Texture1DArray.ArraySize = view.GetLayerCount();
			break;
		case TextureViewType::Texture2D:
			if (isMultisampled)
			{
				desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
			}
			else
			{
				desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
				desc.Texture2D.MipSlice = view.GetBaseMip();
			}
			break;
		case TextureViewType::Texture2DArray:
		case TextureViewType::TextureCube:
		case TextureViewType::TextureCubeArray:
			if (isMultisampled)
			{
				desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
				desc.Texture2DMSArray.FirstArraySlice = view.GetBaseLayer();
				desc.Texture2DMSArray.ArraySize = view.GetLayerCount();
			}
			else
			{
				desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
				desc.Texture2DArray.MipSlice = view.GetBaseMip();
				desc.Texture2DArray.FirstArraySlice = view.GetBaseLayer();
				desc.Texture2DArray.ArraySize = view.GetLayerCount();
			}
			break;
		default:
			break;
		}

		return desc;
	}

	D3D12_DESCRIPTOR_RANGE_TYPE Helpers::ToDescriptorRangeType(DescriptorType type)
	{
		switch (type)
		{
		case DescriptorType::Texture: return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		case DescriptorType::RWTexture: return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
		case DescriptorType::ConstantBuffer: return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		case DescriptorType::Buffer: return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		case DescriptorType::RWBuffer: return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
		case DescriptorType::Sampler: return D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
		default: return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		}
	}

	D3D12_BLEND Helpers::ToBlend(BlendFactor factor)
	{
		switch (factor)
		{
		case BlendFactor::Zero: return D3D12_BLEND_ZERO;
		case BlendFactor::One: return D3D12_BLEND_ONE;
		case BlendFactor::SrcColor: return D3D12_BLEND_SRC_COLOR;
		case BlendFactor::InvSrcColor: return D3D12_BLEND_INV_SRC_COLOR;
		case BlendFactor::SrcAlpha: return D3D12_BLEND_SRC_ALPHA;
		case BlendFactor::InvSrcAlpha: return D3D12_BLEND_INV_SRC_ALPHA;
		case BlendFactor::DstColor: return D3D12_BLEND_DEST_COLOR;
		case BlendFactor::InvDstColor: return D3D12_BLEND_INV_DEST_COLOR;
		case BlendFactor::DstAlpha: return D3D12_BLEND_DEST_ALPHA;
		case BlendFactor::InvDstAlpha: return D3D12_BLEND_INV_DEST_ALPHA;
		case BlendFactor::SrcAlphaSaturate: return D3D12_BLEND_SRC_ALPHA_SAT;
		case BlendFactor::ConstantColor: return D3D12_BLEND_BLEND_FACTOR;
		case BlendFactor::InvConstantColor: return D3D12_BLEND_INV_BLEND_FACTOR;
		default: return D3D12_BLEND_ONE;
		}
	}

	D3D12_BLEND_OP Helpers::ToBlendOp(BlendOp op)
	{
		switch (op)
		{
		case BlendOp::Add: return D3D12_BLEND_OP_ADD;
		case BlendOp::Subtract: return D3D12_BLEND_OP_SUBTRACT;
		case BlendOp::ReverseSubtract: return D3D12_BLEND_OP_REV_SUBTRACT;
		case BlendOp::Min: return D3D12_BLEND_OP_MIN;
		case BlendOp::Max: return D3D12_BLEND_OP_MAX;
		default: return D3D12_BLEND_OP_ADD;
		}
	}

	D3D12_COMPARISON_FUNC Helpers::ToCompareFunc(CompareOp op)
	{
		switch (op)
		{
		case CompareOp::Never: return D3D12_COMPARISON_FUNC_NEVER;
		case CompareOp::Less: return D3D12_COMPARISON_FUNC_LESS;
		case CompareOp::Equal: return D3D12_COMPARISON_FUNC_EQUAL;
		case CompareOp::LessEqual: return D3D12_COMPARISON_FUNC_LESS_EQUAL;
		case CompareOp::Greater: return D3D12_COMPARISON_FUNC_GREATER;
		case CompareOp::NotEqual: return D3D12_COMPARISON_FUNC_NOT_EQUAL;
		case CompareOp::GreaterEqual: return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
		case CompareOp::Always: return D3D12_COMPARISON_FUNC_ALWAYS;
		default: return D3D12_COMPARISON_FUNC_ALWAYS;
		}
	}

	D3D12_STENCIL_OP Helpers::ToStencilOp(StencilOp op)
	{
		switch (op)
		{
		case StencilOp::Keep: return D3D12_STENCIL_OP_KEEP;
		case StencilOp::Zero: return D3D12_STENCIL_OP_ZERO;
		case StencilOp::Replace: return D3D12_STENCIL_OP_REPLACE;
		case StencilOp::IncrSat: return D3D12_STENCIL_OP_INCR_SAT;
		case StencilOp::DecrSat: return D3D12_STENCIL_OP_DECR_SAT;
		case StencilOp::Invert: return D3D12_STENCIL_OP_INVERT;
		case StencilOp::IncrWrap: return D3D12_STENCIL_OP_INCR;
		case StencilOp::DecrWrap: return D3D12_STENCIL_OP_DECR;
		default: return D3D12_STENCIL_OP_KEEP;
		}
	}

	D3D12_FILL_MODE Helpers::ToFillMode(FillMode mode)
	{
		switch (mode)
		{
		case FillMode::Solid: return D3D12_FILL_MODE_SOLID;
		case FillMode::Wireframe: return D3D12_FILL_MODE_WIREFRAME;
		default: return D3D12_FILL_MODE_SOLID;
		}
	}

	D3D12_CULL_MODE Helpers::ToCullMode(CullMode mode)
	{
		switch (mode)
		{
		case CullMode::None: return D3D12_CULL_MODE_NONE;
		case CullMode::Front: return D3D12_CULL_MODE_FRONT;
		case CullMode::Back: return D3D12_CULL_MODE_BACK;
		default: return D3D12_CULL_MODE_BACK;
		}
	}

	D3D12_PRIMITIVE_TOPOLOGY_TYPE Helpers::ToPrimitiveTopologyType(PrimitiveTopology topology)
	{
		switch (topology)
		{
		case PrimitiveTopology::PointList: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
		case PrimitiveTopology::LineList: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		case PrimitiveTopology::LineStrip: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		case PrimitiveTopology::TriangleList: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		case PrimitiveTopology::TriangleStrip: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		default: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		}
	}

	D3D12_PRIMITIVE_TOPOLOGY Helpers::ToPrimitiveTopology(PrimitiveTopology topology)
	{
		switch (topology)
		{
		case PrimitiveTopology::PointList: return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
		case PrimitiveTopology::LineList: return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
		case PrimitiveTopology::LineStrip: return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
		case PrimitiveTopology::TriangleList: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		case PrimitiveTopology::TriangleStrip: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		default: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		}
	}

	D3D12_RASTERIZER_DESC Helpers::ToRasterizerDesc(const RasterizerState& state)
	{
		D3D12_RASTERIZER_DESC desc = {};
		desc.FillMode = ToFillMode(state.fillMode);
		desc.CullMode = ToCullMode(state.cullMode);
		desc.FrontCounterClockwise = state.frontFace == FrontFace::CCW;
		desc.DepthBias = state.depthBias;
		desc.DepthBiasClamp = state.depthBiasClamp;
		desc.SlopeScaledDepthBias = state.slopeScaledDepthBias;
		desc.DepthClipEnable = state.depthClip;
		desc.MultisampleEnable = FALSE;
		desc.AntialiasedLineEnable = FALSE;
		desc.ForcedSampleCount = 0;
		desc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
		return desc;
	}

	D3D12_DEPTH_STENCIL_DESC Helpers::ToDepthStencilDesc(const DepthStencilState& state)
	{
		D3D12_DEPTH_STENCIL_DESC desc = {};
		desc.DepthEnable = state.depthTest;
		desc.DepthWriteMask = state.depthWrite ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
		desc.DepthFunc = ToCompareFunc(state.depthCompare);

		desc.StencilEnable = state.stencilTest;
		desc.StencilReadMask = state.stencilReadMask;
		desc.StencilWriteMask = state.stencilWriteMask;

		desc.FrontFace.StencilFailOp = ToStencilOp(state.front.failOp);
		desc.FrontFace.StencilDepthFailOp = ToStencilOp(state.front.depthFailOp);
		desc.FrontFace.StencilPassOp = ToStencilOp(state.front.passOp);
		desc.FrontFace.StencilFunc = ToCompareFunc(state.front.compareOp);

		desc.BackFace.StencilFailOp = ToStencilOp(state.back.failOp);
		desc.BackFace.StencilDepthFailOp = ToStencilOp(state.back.depthFailOp);
		desc.BackFace.StencilPassOp = ToStencilOp(state.back.passOp);
		desc.BackFace.StencilFunc = ToCompareFunc(state.back.compareOp);

		return desc;
	}

	D3D12_BLEND_DESC Helpers::ToBlendDesc(ReadOnlyList<ColorTarget> targets, b8 alphaToCoverage)
	{
		D3D12_BLEND_DESC desc = {};
		desc.AlphaToCoverageEnable = alphaToCoverage;
		desc.IndependentBlendEnable = TRUE;

		for (usize i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
		{
			BlendState blend = i < targets.GetCount() ? targets[i].blend : BlendState();

			D3D12_RENDER_TARGET_BLEND_DESC& target = desc.RenderTarget[i];
			target.BlendEnable = blend.enable;
			target.LogicOpEnable = FALSE;
			target.SrcBlend = ToBlend(blend.srcColor);
			target.DestBlend = ToBlend(blend.dstColor);
			target.BlendOp = ToBlendOp(blend.colorOp);
			target.SrcBlendAlpha = ToBlend(blend.srcAlpha);
			target.DestBlendAlpha = ToBlend(blend.dstAlpha);
			target.BlendOpAlpha = ToBlendOp(blend.alphaOp);
			target.LogicOp = D3D12_LOGIC_OP_NOOP;
			target.RenderTargetWriteMask = static_cast<UINT8>(blend.writeMask);
		}

		return desc;
	}

	D3D12_COMMAND_LIST_TYPE Helpers::ToCommandListType(QueueType type)
	{
		switch (type)
		{
		case QueueType::Graphics: return D3D12_COMMAND_LIST_TYPE_DIRECT;
		case QueueType::Compute: return D3D12_COMMAND_LIST_TYPE_COMPUTE;
		case QueueType::Transfer: return D3D12_COMMAND_LIST_TYPE_COPY;
		default: return D3D12_COMMAND_LIST_TYPE_DIRECT;
		}
	}

	DXGI_FORMAT Helpers::ToIndexFormat(IndexType type)
	{
		switch (type)
		{
		case IndexType::Index16: return DXGI_FORMAT_R16_UINT;
		case IndexType::Index32: return DXGI_FORMAT_R32_UINT;
		default: return DXGI_FORMAT_R32_UINT;
		}
	}

	D3D12_BARRIER_SYNC Helpers::ToBarrierSync(ResourceState state)
	{
		if (state == ResourceState::Undefined || state == ResourceState::Present)
			return D3D12_BARRIER_SYNC_NONE;

		D3D12_BARRIER_SYNC sync = D3D12_BARRIER_SYNC_NONE;

		if (HasFlag(state, ResourceState::Common))
			sync |= D3D12_BARRIER_SYNC_ALL;

		if (HasFlag(state, ResourceState::VertexBuffer))
			sync |= D3D12_BARRIER_SYNC_VERTEX_SHADING;

		if (HasFlag(state, ResourceState::IndexBuffer))
			sync |= D3D12_BARRIER_SYNC_INDEX_INPUT;

		if (HasFlag(state, ResourceState::ConstantBuffer | ResourceState::ShaderResource | ResourceState::UnorderedAccess))
			sync |= D3D12_BARRIER_SYNC_ALL_SHADING;

		if (HasFlag(state, ResourceState::IndirectArgument))
			sync |= D3D12_BARRIER_SYNC_EXECUTE_INDIRECT;

		if (HasFlag(state, ResourceState::RenderTarget))
			sync |= D3D12_BARRIER_SYNC_RENDER_TARGET;

		if (HasFlag(state, ResourceState::DepthWrite | ResourceState::DepthRead))
			sync |= D3D12_BARRIER_SYNC_DEPTH_STENCIL;

		if (HasFlag(state, ResourceState::CopySrc | ResourceState::CopyDst))
			sync |= D3D12_BARRIER_SYNC_COPY;

		if (HasFlag(state, ResourceState::ResolveSrc | ResourceState::ResolveDst))
			sync |= D3D12_BARRIER_SYNC_RESOLVE;

		return sync;
	}

	D3D12_BARRIER_ACCESS Helpers::ToBarrierAccess(ResourceState state)
	{
		if (state == ResourceState::Undefined || state == ResourceState::Present)
			return D3D12_BARRIER_ACCESS_NO_ACCESS;

		D3D12_BARRIER_ACCESS access = D3D12_BARRIER_ACCESS_COMMON;

		if (HasFlag(state, ResourceState::VertexBuffer))
			access |= D3D12_BARRIER_ACCESS_VERTEX_BUFFER;

		if (HasFlag(state, ResourceState::IndexBuffer))
			access |= D3D12_BARRIER_ACCESS_INDEX_BUFFER;

		if (HasFlag(state, ResourceState::ConstantBuffer))
			access |= D3D12_BARRIER_ACCESS_CONSTANT_BUFFER;

		if (HasFlag(state, ResourceState::IndirectArgument))
			access |= D3D12_BARRIER_ACCESS_INDIRECT_ARGUMENT;

		if (HasFlag(state, ResourceState::ShaderResource))
			access |= D3D12_BARRIER_ACCESS_SHADER_RESOURCE;

		if (HasFlag(state, ResourceState::UnorderedAccess))
			access |= D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;

		if (HasFlag(state, ResourceState::RenderTarget))
			access |= D3D12_BARRIER_ACCESS_RENDER_TARGET;

		if (HasFlag(state, ResourceState::DepthWrite))
			access |= D3D12_BARRIER_ACCESS_DEPTH_STENCIL_WRITE;

		if (HasFlag(state, ResourceState::DepthRead))
			access |= D3D12_BARRIER_ACCESS_DEPTH_STENCIL_READ;

		if (HasFlag(state, ResourceState::CopySrc))
			access |= D3D12_BARRIER_ACCESS_COPY_SOURCE;

		if (HasFlag(state, ResourceState::CopyDst))
			access |= D3D12_BARRIER_ACCESS_COPY_DEST;

		if (HasFlag(state, ResourceState::ResolveSrc))
			access |= D3D12_BARRIER_ACCESS_RESOLVE_SOURCE;

		if (HasFlag(state, ResourceState::ResolveDst))
			access |= D3D12_BARRIER_ACCESS_RESOLVE_DEST;

		return access;
	}

	D3D12_BARRIER_LAYOUT Helpers::ToBarrierLayout(ResourceState state)
	{
		if (state == ResourceState::Undefined)
			return D3D12_BARRIER_LAYOUT_UNDEFINED;

		if (HasFlag(state, ResourceState::Common))
			return D3D12_BARRIER_LAYOUT_COMMON;

		if (HasFlag(state, ResourceState::Present))
			return D3D12_BARRIER_LAYOUT_PRESENT;

		if (HasFlag(state, ResourceState::RenderTarget))
			return D3D12_BARRIER_LAYOUT_RENDER_TARGET;

		if (HasFlag(state, ResourceState::DepthWrite))
			return D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE;

		u32 readCount = 0;
		if (HasFlag(state, ResourceState::DepthRead))
			readCount++;
		if (HasFlag(state, ResourceState::ShaderResource))
			readCount++;
		if (HasFlag(state, ResourceState::CopySrc))
			readCount++;
		if (HasFlag(state, ResourceState::ResolveSrc))
			readCount++;

		if (readCount > 1)
			return D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_GENERIC_READ;

		if (HasFlag(state, ResourceState::DepthRead))
			return D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_READ;

		if (HasFlag(state, ResourceState::UnorderedAccess))
			return D3D12_BARRIER_LAYOUT_UNORDERED_ACCESS;

		if (HasFlag(state, ResourceState::CopyDst))
			return D3D12_BARRIER_LAYOUT_COPY_DEST;

		if (HasFlag(state, ResourceState::CopySrc))
			return D3D12_BARRIER_LAYOUT_COPY_SOURCE;

		if (HasFlag(state, ResourceState::ShaderResource))
			return D3D12_BARRIER_LAYOUT_SHADER_RESOURCE;

		if (HasFlag(state, ResourceState::ResolveDst))
			return D3D12_BARRIER_LAYOUT_RESOLVE_DEST;

		if (HasFlag(state, ResourceState::ResolveSrc))
			return D3D12_BARRIER_LAYOUT_RESOLVE_SOURCE;

		return D3D12_BARRIER_LAYOUT_COMMON;
	}

	D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE Helpers::ToBeginningAccess(LoadOp op)
	{
		switch (op)
		{
		case LoadOp::Load: return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE;
		case LoadOp::Clear: return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
		case LoadOp::DontCare: return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_DISCARD;
		default: return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE;
		}
	}

	D3D12_RENDER_PASS_ENDING_ACCESS_TYPE Helpers::ToEndingAccess(StoreOp op)
	{
		switch (op)
		{
		case StoreOp::Store: return D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
		case StoreOp::DontCare: return D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD;
		default: return D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
		}
	}

	D3D12_TEXTURE_ADDRESS_MODE Helpers::ToAddressMode(SamplerAddressMode mode)
	{
		switch (mode)
		{
		case SamplerAddressMode::Repeat: return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		case SamplerAddressMode::MirroredRepeat: return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
		case SamplerAddressMode::ClampToEdge: return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		case SamplerAddressMode::ClampToBorder: return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		case SamplerAddressMode::MirrorClampToEdge: return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
		default: return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		}
	}

	D3D12_SAMPLER_DESC Helpers::ToSamplerDesc(const SamplerDesc& desc)
	{
		D3D12_FILTER_REDUCTION_TYPE reduction = desc.compareEnable ? D3D12_FILTER_REDUCTION_TYPE_COMPARISON : D3D12_FILTER_REDUCTION_TYPE_STANDARD;

		D3D12_SAMPLER_DESC samplerDesc = {};

		if (desc.maxAnisotropy > 1)
		{
			samplerDesc.Filter = D3D12_ENCODE_ANISOTROPIC_FILTER(reduction);
		}
		else
		{
			D3D12_FILTER_TYPE minFilter = desc.minFilter == SamplerFilter::Linear ? D3D12_FILTER_TYPE_LINEAR : D3D12_FILTER_TYPE_POINT;
			D3D12_FILTER_TYPE magFilter = desc.magFilter == SamplerFilter::Linear ? D3D12_FILTER_TYPE_LINEAR : D3D12_FILTER_TYPE_POINT;
			D3D12_FILTER_TYPE mipFilter = desc.mipFilter == SamplerFilter::Linear ? D3D12_FILTER_TYPE_LINEAR : D3D12_FILTER_TYPE_POINT;
			samplerDesc.Filter = D3D12_ENCODE_BASIC_FILTER(minFilter, magFilter, mipFilter, reduction);
		}

		samplerDesc.AddressU = ToAddressMode(desc.addressU);
		samplerDesc.AddressV = ToAddressMode(desc.addressV);
		samplerDesc.AddressW = ToAddressMode(desc.addressW);
		samplerDesc.MipLODBias = desc.mipLodBias;
		samplerDesc.MaxAnisotropy = desc.maxAnisotropy > 16 ? 16 : desc.maxAnisotropy;
		samplerDesc.ComparisonFunc = desc.compareEnable ? ToCompareFunc(desc.compareOp) : D3D12_COMPARISON_FUNC_NEVER;
		samplerDesc.MinLOD = desc.minLod;
		samplerDesc.MaxLOD = desc.maxLod;

		f32 borderAlpha = desc.borderColor == SamplerBorderColor::TransparentBlack ? 0.0f : 1.0f;
		f32 borderValue = desc.borderColor == SamplerBorderColor::OpaqueWhite ? 1.0f : 0.0f;
		samplerDesc.BorderColor[0] = borderValue;
		samplerDesc.BorderColor[1] = borderValue;
		samplerDesc.BorderColor[2] = borderValue;
		samplerDesc.BorderColor[3] = borderAlpha;

		return samplerDesc;
	}
}