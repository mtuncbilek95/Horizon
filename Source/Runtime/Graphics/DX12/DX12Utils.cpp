#include "DX12Utils.h"

namespace Horizon
{
	DXGI_FORMAT ToDXGI(GfxTextureFormat fmt)
	{
		switch (fmt)
		{
		case GfxTextureFormat::R8:         return DXGI_FORMAT_R8_UNORM;
		case GfxTextureFormat::RG8:        return DXGI_FORMAT_R8G8_UNORM;
		case GfxTextureFormat::RGBA8:      return DXGI_FORMAT_R8G8B8A8_UNORM;
		case GfxTextureFormat::RGBA8_sRGB: return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		case GfxTextureFormat::BGRA8:      return DXGI_FORMAT_B8G8R8A8_UNORM;
		case GfxTextureFormat::BGRA8_sRGB: return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
		case GfxTextureFormat::R16F:       return DXGI_FORMAT_R16_FLOAT;
		case GfxTextureFormat::RG16F:      return DXGI_FORMAT_R16G16_FLOAT;
		case GfxTextureFormat::RGBA16F:    return DXGI_FORMAT_R16G16B16A16_FLOAT;
		case GfxTextureFormat::R32F:       return DXGI_FORMAT_R32_FLOAT;
		case GfxTextureFormat::RG32F:      return DXGI_FORMAT_R32G32_FLOAT;
		case GfxTextureFormat::RGBA32F:    return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case GfxTextureFormat::R11G11B10F: return DXGI_FORMAT_R11G11B10_FLOAT;
		case GfxTextureFormat::RGB10A2:    return DXGI_FORMAT_R10G10B10A2_UNORM;
		case GfxTextureFormat::R32U:       return DXGI_FORMAT_R32_UINT;
		case GfxTextureFormat::RG32U:      return DXGI_FORMAT_R32G32_UINT;
		case GfxTextureFormat::D16:        return DXGI_FORMAT_D16_UNORM;
		case GfxTextureFormat::D32:        return DXGI_FORMAT_D32_FLOAT;
		case GfxTextureFormat::D24S8:      return DXGI_FORMAT_D24_UNORM_S8_UINT;
		case GfxTextureFormat::BC7:        return DXGI_FORMAT_BC7_UNORM;
		case GfxTextureFormat::BC7_sRGB:   return DXGI_FORMAT_BC7_UNORM_SRGB;
		default:                           return DXGI_FORMAT_UNKNOWN;
		}
	}

	b8 IsDepthFormat(GfxTextureFormat fmt)
	{
		return fmt == GfxTextureFormat::D16 || fmt == GfxTextureFormat::D32 || fmt == GfxTextureFormat::D24S8;
	}

	DXGI_FORMAT ToTypeless(GfxTextureFormat fmt)
	{
		switch (fmt)
		{
		case GfxTextureFormat::D16:   return DXGI_FORMAT_R16_TYPELESS;
		case GfxTextureFormat::D32:   return DXGI_FORMAT_R32_TYPELESS;
		case GfxTextureFormat::D24S8: return DXGI_FORMAT_R24G8_TYPELESS;
		default:                      return ToDXGI(fmt);
		}
	}

	DXGI_FORMAT ToShaderViewFormat(GfxTextureFormat fmt)
	{
		switch (fmt)
		{
		case GfxTextureFormat::D16:   return DXGI_FORMAT_R16_UNORM;
		case GfxTextureFormat::D32:   return DXGI_FORMAT_R32_FLOAT;
		case GfxTextureFormat::D24S8: return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		default:                      return ToDXGI(fmt);
		}
	}

	D3D12_RESOURCE_STATES ToResourceState(GfxResourceState state)
	{
		switch (state)
		{
		case GfxResourceState::Common:          return D3D12_RESOURCE_STATE_COMMON;
		case GfxResourceState::VertexBuffer:    return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
		case GfxResourceState::IndexBuffer:     return D3D12_RESOURCE_STATE_INDEX_BUFFER;
		case GfxResourceState::ConstantBuffer:  return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
		case GfxResourceState::IndirectArg:     return D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
		case GfxResourceState::ShaderResource:  return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		case GfxResourceState::UnorderedAccess: return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		case GfxResourceState::RenderTarget:    return D3D12_RESOURCE_STATE_RENDER_TARGET;
		case GfxResourceState::DepthWrite:      return D3D12_RESOURCE_STATE_DEPTH_WRITE;
		case GfxResourceState::DepthRead:       return D3D12_RESOURCE_STATE_DEPTH_READ;
		case GfxResourceState::CopySrc:         return D3D12_RESOURCE_STATE_COPY_SOURCE;
		case GfxResourceState::CopyDst:         return D3D12_RESOURCE_STATE_COPY_DEST;
		case GfxResourceState::Present:         return D3D12_RESOURCE_STATE_PRESENT;
		case GfxResourceState::Undefined:
		default:                                return D3D12_RESOURCE_STATE_COMMON;
		}
	}
}