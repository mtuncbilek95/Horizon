#include "D3D12Backend.h"

#include <Runtime/Graphics/GfxBackend.h>

namespace Horizon
{
	GfxTexture* Gfx::CreateGfxTexture(GfxDevice* pContext, const GfxTextureDesc& desc)
	{
		const DXGI_FORMAT viewFormat = Helpers::ToDXGIFormat(desc.format);
		const b8 isDepth = Helpers::IsDepthFormat(desc.format);
		const b8 isSampled = has(desc.usage, GfxTextureUsage::Sampled);

		D3D12_RESOURCE_DESC resourceDesc = {};
		resourceDesc.Dimension = desc.type == GfxTextureType::Tex3D
			? D3D12_RESOURCE_DIMENSION_TEXTURE3D : D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resourceDesc.Width = desc.width;
		resourceDesc.Height = desc.height;
		resourceDesc.DepthOrArraySize = u16(desc.depth);
		resourceDesc.MipLevels = u16(desc.mipLevels);
		resourceDesc.Format = (isDepth && isSampled) ? Helpers::ToTypelessFormat(viewFormat) : viewFormat;
		resourceDesc.SampleDesc = { 1, 0 };
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

		if (has(desc.usage, GfxTextureUsage::RenderTarget))
			resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		if (has(desc.usage, GfxTextureUsage::DepthStencil))
			resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		if (has(desc.usage, GfxTextureUsage::Storage))
			resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		if (isDepth && !isSampled)
			resourceDesc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;

		D3D12_CLEAR_VALUE clearValue = {};
		clearValue.Format = viewFormat;
		if (isDepth) { clearValue.DepthStencil = { 1.0f, 0 }; }
		else { clearValue.Color[3] = 1.0f; }

		const b8 wantsClear = has(desc.usage, GfxTextureUsage::RenderTarget)
			|| has(desc.usage, GfxTextureUsage::DepthStencil);

		D3D12MA::ALLOCATION_DESC allocDesc = {};
		allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

		GfxTexture* pTexture = new GfxTexture();
		HRESULT bResult = pContext->pAllocator->CreateResource(&allocDesc, &resourceDesc,
			D3D12_RESOURCE_STATE_COMMON, wantsClear ? &clearValue : nullptr,
			&pTexture->pMemory, IID_PPV_ARGS(&pTexture->pResource));
		CHECK_REASON(bResult, "ID3D12Texture - CreateResource");

		if (FAILED(bResult))
		{
			delete pTexture;
			return nullptr;
		}

		pTexture->format = viewFormat;
		pTexture->type = desc.type;
		pTexture->width = desc.width;
		pTexture->height = desc.height;
		pTexture->depth = desc.depth;
		pTexture->mipLevels = desc.mipLevels;

		return pTexture;
	}

	void Gfx::DestroyGfxTexture(GfxTexture* texHandl)
	{
		if (texHandl->pResource)
			texHandl->pResource->Release();
		if (texHandl->pMemory)
			texHandl->pMemory->Release();

		delete texHandl;
	}

	u32 Gfx::CreateTextureSRV(GfxDevice* pContext, GfxDescriptorHeap* pHeap, GfxTexture* pTexture)
	{
		u32 index = Helpers::AllocateDescriptor(pHeap);

		D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
		viewDesc.Format = Helpers::ToDepthSRVFormat(pTexture->format);
		viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		switch (pTexture->type)
		{
		case GfxTextureType::Tex2D:
			viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			viewDesc.Texture2D.MipLevels = pTexture->mipLevels;
			break;
		case GfxTextureType::Tex2DArray:
			viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
			viewDesc.Texture2DArray.MipLevels = pTexture->mipLevels;
			viewDesc.Texture2DArray.ArraySize = pTexture->depth;
			break;
		case GfxTextureType::Tex3D:
			viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
			viewDesc.Texture3D.MipLevels = pTexture->mipLevels;
			break;
		}

		pContext->pDevice->CreateShaderResourceView(pTexture->pResource, &viewDesc,
			Helpers::CpuAt(pHeap, index));

		pTexture->shaderViewIndex = index;
		return index;
	}

	u32 Gfx::CreateTextureRTV(GfxDevice* pContext, GfxDescriptorHeap* pHeap, GfxTexture* pTexture)
	{
		u32 index = Helpers::AllocateDescriptor(pHeap);
		pContext->pDevice->CreateRenderTargetView(pTexture->pResource, nullptr,
			Helpers::CpuAt(pHeap, index));

		pTexture->targetViewIndex = index;
		pTexture->targetViewHandle = Helpers::CpuAt(pHeap, index);
		return index;
	}

	u32 Gfx::CreateTextureDSV(GfxDevice* pContext, GfxDescriptorHeap* pHeap, GfxTexture* pTexture)
	{
		u32 index = Helpers::AllocateDescriptor(pHeap);

		D3D12_DEPTH_STENCIL_VIEW_DESC viewDesc = {};
		viewDesc.Format = pTexture->format;
		viewDesc.ViewDimension = pTexture->type == GfxTextureType::Tex2DArray
			? D3D12_DSV_DIMENSION_TEXTURE2DARRAY : D3D12_DSV_DIMENSION_TEXTURE2D;
		if (pTexture->type == GfxTextureType::Tex2DArray)
			viewDesc.Texture2DArray.ArraySize = pTexture->depth;

		pContext->pDevice->CreateDepthStencilView(pTexture->pResource, &viewDesc,
			Helpers::CpuAt(pHeap, index));

		pTexture->depthViewIndex = index;
		pTexture->depthViewHandle = Helpers::CpuAt(pHeap, index);
		return index;
	}

	u32 Gfx::GetTextureShaderView(const GfxTexture* pTexture)
	{
		return pTexture->shaderViewIndex;
	}

	u32 Gfx::GetTextureAccessView(const GfxTexture* pTexture)
	{
		return pTexture->accessViewIndex;
	}

	u32 Gfx::GetTextureTargetView(const GfxTexture* pTexture)
	{
		return pTexture->targetViewIndex;
	}

	u32 Gfx::GetTextureDepthView(const GfxTexture* pTexture)
	{
		return pTexture->depthViewIndex;
	}

	u32 Gfx::CreateTextureUAV(GfxDevice* pContext, GfxDescriptorHeap* pHeap, GfxTexture* pTexture)
	{
		u32 index = Helpers::AllocateDescriptor(pHeap);

		D3D12_UNORDERED_ACCESS_VIEW_DESC viewDesc = {};
		viewDesc.Format = pTexture->format;
		viewDesc.ViewDimension = pTexture->type == GfxTextureType::Tex3D
			? D3D12_UAV_DIMENSION_TEXTURE3D : D3D12_UAV_DIMENSION_TEXTURE2D;
		if (pTexture->type == GfxTextureType::Tex3D)
			viewDesc.Texture3D.WSize = pTexture->depth;

		pContext->pDevice->CreateUnorderedAccessView(pTexture->pResource, nullptr, &viewDesc,
			Helpers::CpuAt(pHeap, index));

		pTexture->accessViewIndex = index;
		return index;
	}
}