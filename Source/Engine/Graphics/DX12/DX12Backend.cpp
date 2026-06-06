#include "DX12Backend.h"

namespace Horizon::DX12
{
	DXGI_FORMAT ToDXGI(GfxTextureFormat format)
	{
		switch (format)
		{
		case GfxTextureFormat::R8:
			return DXGI_FORMAT_R8_UNORM;
		case GfxTextureFormat::RG8:
			return DXGI_FORMAT_R8G8_UNORM;
		case GfxTextureFormat::RGBA8:
			return DXGI_FORMAT_R8G8B8A8_UNORM;
		case GfxTextureFormat::RGBA8_sRGB:
			return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		case GfxTextureFormat::BGRA8:
			return DXGI_FORMAT_B8G8R8A8_UNORM;
		case GfxTextureFormat::BGRA8_sRGB:
			return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
		case GfxTextureFormat::R16F:
			return DXGI_FORMAT_R16_FLOAT;
		case GfxTextureFormat::RG16F:
			return DXGI_FORMAT_R16G16_FLOAT;
		case GfxTextureFormat::RGBA16F:
			return DXGI_FORMAT_R16G16B16A16_FLOAT;
		case GfxTextureFormat::R32F:
			return DXGI_FORMAT_R32_FLOAT;
		case GfxTextureFormat::RG32F:
			return DXGI_FORMAT_R32G32_FLOAT;
		case GfxTextureFormat::RGBA32F:
			return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case GfxTextureFormat::R11G11B10F:
			return DXGI_FORMAT_R11G11B10_FLOAT;
		case GfxTextureFormat::RGB10A2:
			return DXGI_FORMAT_R10G10B10A2_UNORM;
		case GfxTextureFormat::R32U:
			return DXGI_FORMAT_R32_UINT;
		case GfxTextureFormat::RG32U:
			return DXGI_FORMAT_R32G32_UINT;
		case GfxTextureFormat::D16:
			return DXGI_FORMAT_D16_UNORM;
		case GfxTextureFormat::D32:
			return DXGI_FORMAT_D32_FLOAT;
		case GfxTextureFormat::D24S8:
			return DXGI_FORMAT_D24_UNORM_S8_UINT;
		default:
			return DXGI_FORMAT_R8G8B8A8_UNORM;
		}
	}

	b8 IsDepthFormat(GfxTextureFormat format)
	{
		return format == GfxTextureFormat::D16 || format == GfxTextureFormat::D32 || format == GfxTextureFormat::D24S8;
	}

	DXGI_FORMAT ToTypeless(GfxTextureFormat format)
	{
		switch (format)
		{
		case GfxTextureFormat::D16:
			return DXGI_FORMAT_R16_TYPELESS;
		case GfxTextureFormat::D32:
			return DXGI_FORMAT_R32_TYPELESS;
		case GfxTextureFormat::D24S8:
			return DXGI_FORMAT_R24G8_TYPELESS;
		default:
			return ToDXGI(format);
		}
	}

	DXGI_FORMAT ToShaderViewFormat(GfxTextureFormat format)
	{
		switch (format)
		{
		case GfxTextureFormat::D16:
			return DXGI_FORMAT_R16_UNORM;
		case GfxTextureFormat::D32:
			return DXGI_FORMAT_R32_FLOAT;
		case GfxTextureFormat::D24S8:
			return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		default:
			return ToDXGI(format);
		}
	}

	void CreateStaging(u64 size, ID3D12Resource** resource, D3D12MA::Allocation** allocation)
	{
		Context& context = GfxContext();

		D3D12_RESOURCE_DESC resourceDesc = {};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Width = size;
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		resourceDesc.SampleDesc = { 1, 0 };
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		D3D12MA::ALLOCATION_DESC allocDesc = {};
		allocDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;

		context.allocator->CreateResource(&allocDesc, &resourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, allocation, IID_PPV_ARGS(resource));
	}

	void RawTransition(ID3D12GraphicsCommandList* list, ID3D12Resource* resource,
		D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
	{
		if (before == after)
			return;

		D3D12_RESOURCE_BARRIER barrierVal = {};
		barrierVal.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrierVal.Transition.pResource = resource;
		barrierVal.Transition.StateBefore = before;
		barrierVal.Transition.StateAfter = after;
		barrierVal.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		list->ResourceBarrier(1, &barrierVal);
	}

	void SetDebugName(ID3D12Object* object, std::string_view name)
	{
		if (!object || name.empty())
			return;

		std::array<wchar_t, 128> wide;
		if (MultiByteToWideChar(CP_UTF8, 0, name.data(), -1, wide.data(), 128) > 0)
			object->SetName(wide.data());
	}
}