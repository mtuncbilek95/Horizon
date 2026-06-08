#include "DX12Context.h"

namespace Horizon
{
	GfxTextureHandle GfxDevice::CreateTexture(const GfxTextureDesc& desc)
	{
		Context& context = DX12Context();

		const b8 isDepth = (desc.format == GfxTextureFormat::D16 ||
							desc.format == GfxTextureFormat::D32 ||
							desc.format == GfxTextureFormat::D24S8);

		DXGI_FORMAT resFormat = Helpers::ToDXGIFormat(desc.format);
		DXGI_FORMAT dsvFormat = resFormat;
		DXGI_FORMAT srvFormat = resFormat;
		if (isDepth)
		{
			switch (desc.format)
			{
			case GfxTextureFormat::D16:
				resFormat = DXGI_FORMAT_R16_TYPELESS;
				dsvFormat = DXGI_FORMAT_D16_UNORM;
				srvFormat = DXGI_FORMAT_R16_UNORM;
				break;
			case GfxTextureFormat::D32:
				resFormat = DXGI_FORMAT_R32_TYPELESS;
				dsvFormat = DXGI_FORMAT_D32_FLOAT;
				srvFormat = DXGI_FORMAT_R32_FLOAT;
				break;
			case GfxTextureFormat::D24S8:
				resFormat = DXGI_FORMAT_R24G8_TYPELESS;
				dsvFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
				srvFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
				break;
			default:
				break;
			}
		}

		D3D12_RESOURCE_DESC resDesc = {};
		resDesc.Dimension = (desc.type == GfxTextureType::Tex3D) ? D3D12_RESOURCE_DIMENSION_TEXTURE3D
																 : D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resDesc.Width = desc.width;
		resDesc.Height = desc.height;
		resDesc.DepthOrArraySize = u16(desc.type == GfxTextureType::Tex2D ? 1 : desc.depth);
		resDesc.MipLevels = u16(desc.mipLevels);
		resDesc.Format = resFormat;
		resDesc.SampleDesc = { 1, 0 };
		resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		if (has(desc.usage, GfxTextureUsage::RenderTarget))
			resDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

		if (has(desc.usage, GfxTextureUsage::DepthStencil))
			resDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		if (has(desc.usage, GfxTextureUsage::Storage))
			resDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

		D3D12_CLEAR_VALUE clearValue = {};
		D3D12_CLEAR_VALUE* pClearValue = nullptr;
		if (has(desc.usage, GfxTextureUsage::RenderTarget))
		{
			clearValue.Format = resFormat;
			pClearValue = &clearValue;
		}
		else if (has(desc.usage, GfxTextureUsage::DepthStencil))
		{
			clearValue.Format = dsvFormat;
			clearValue.DepthStencil = { 1.0f, 0 };
			pClearValue = &clearValue;
		}

		D3D12_RESOURCE_STATES initState = D3D12_RESOURCE_STATE_COMMON;
		if (has(desc.usage, GfxTextureUsage::DepthStencil))
		{
			initState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
		}

		D3D12MA::ALLOCATION_DESC allocDesc = {};
		allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

		DX12Texture texture = {};
		texture.format = resFormat;
		texture.width = desc.width;
		texture.height = desc.height;
		texture.state = initState;

		HRESULT bResult = context.pAllocator->CreateResource(&allocDesc, &resDesc, initState, pClearValue,
			&texture.pMemory, IID_PPV_ARGS(&texture.pResource));
		CHECK_REASON(bResult, "ID3D12Resource - CreateResource (texture)");

		if (FAILED(bResult))
		{
			return GfxTextureHandle();
		}

		if (has(desc.usage, GfxTextureUsage::Sampled))
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = srvFormat;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			switch (desc.type)
			{
			case GfxTextureType::Tex3D:
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
				srvDesc.Texture3D.MipLevels = desc.mipLevels;
				break;
			case GfxTextureType::Tex2DArray:
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
				srvDesc.Texture2DArray.MipLevels = desc.mipLevels;
				srvDesc.Texture2DArray.ArraySize = desc.depth;
				break;
			default:
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				srvDesc.Texture2D.MipLevels = desc.mipLevels;
				break;
			}
			texture.shaderViewIndex = Helpers::AllocateDescriptor(context.resourceHeap);
			context.pDevice->CreateShaderResourceView(texture.pResource, &srvDesc,
				Helpers::CpuAt(context.resourceHeap, texture.shaderViewIndex));
		}

		if (has(desc.usage, GfxTextureUsage::Storage))
		{
			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			uavDesc.Format = Helpers::ToDXGIFormat(desc.format);
			switch (desc.type)
			{
			case GfxTextureType::Tex3D:
				uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
				uavDesc.Texture3D.WSize = desc.depth;
				break;
			case GfxTextureType::Tex2DArray:
				uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
				uavDesc.Texture2DArray.ArraySize = desc.depth;
				break;
			default:
				uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
				break;
			}
			texture.accessViewIndex = Helpers::AllocateDescriptor(context.resourceHeap);
			context.pDevice->CreateUnorderedAccessView(texture.pResource, nullptr, &uavDesc,
				Helpers::CpuAt(context.resourceHeap, texture.accessViewIndex));
		}

		if (has(desc.usage, GfxTextureUsage::RenderTarget))
		{
			texture.targetViewIndex = Helpers::AllocateDescriptor(context.renderTargetHeap);
			context.pDevice->CreateRenderTargetView(texture.pResource, nullptr,
				Helpers::CpuAt(context.renderTargetHeap, texture.targetViewIndex));
		}

		if (has(desc.usage, GfxTextureUsage::DepthStencil))
		{
			D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
			dsvDesc.Format = dsvFormat;
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			texture.depthViewIndex = Helpers::AllocateDescriptor(context.depthStencilHeap);
			context.pDevice->CreateDepthStencilView(texture.pResource, &dsvDesc,
				Helpers::CpuAt(context.depthStencilHeap, texture.depthViewIndex));
		}

		if (!desc.debugName.empty())
		{
			texture.pResource->SetPrivateData(WKPDID_D3DDebugObjectName, u32(desc.debugName.size()), desc.debugName.data());
		}

		return context.texturePool.Insert(std::move(texture));
	}

	void GfxDevice::WriteTexture(GfxTextureHandle handle, void* pData, usize sizeInBytes, usize offset)
	{}

	void GfxDevice::DestroyTexture(GfxTextureHandle handle)
	{}
}