#include "DX12Backend.h"

#include <Engine/Log/Log.h>
#include <Engine/Graphics/RHI/GfxTexture.h>

#define CHECK_REASON(hr, what) \
	if (FAILED(hr)) { ConsoleLog().Error("{}: {}", what, _com_error(hr).ErrorMessage()); }

namespace Horizon::DX12
{
	GfxTextureHandle TexturePoolAlloc()
	{
		Context& context = GfxContext();
		u32 index;
		if (!context.texFreeList.empty())
		{
			index = context.texFreeList.back();
			context.texFreeList.pop_back();
		}
		else
		{
			index = u32(context.textures.size());
			context.textures.push_back({});
			context.texGenerations.push_back(0);
		}

		return GfxTextureHandle::make(index, context.texGenerations[index]);
	}

	DX12Texture& TexturePoolGet(GfxTextureHandle handle) { return GfxContext().textures[handle.index()]; }

	void TexturePoolFree(GfxTextureHandle handle)
	{
		Context& context = GfxContext();
		context.textures[handle.index()] = {};
		context.texGenerations[handle.index()]++;
		context.texFreeList.push_back(handle.index());
	}
}

namespace Horizon
{
	using namespace DX12;

	GfxTextureHandle GfxDevice::CreateTexture(const GfxTextureDesc& desc)
	{
		Context& context = GfxContext();

		const b8 sampled = has(desc.usage, GfxTextureUsage::Sampled);
		const b8 isRT = has(desc.usage, GfxTextureUsage::RenderTarget);
		const b8 isDS = has(desc.usage, GfxTextureUsage::DepthStencil);
		const b8 isUAV = has(desc.usage, GfxTextureUsage::Storage);
		const b8 depthFmt = IsDepthFormat(desc.format);

		const b8 is3D = desc.type == GfxTextureType::Tex3D;
		const b8 isArray = desc.type == GfxTextureType::Tex2DArray;

		D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;
		if (isRT)
			flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		if (isDS)
			flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		if (isUAV)
			flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

		DXGI_FORMAT resFormat = (depthFmt && sampled) ? ToTypeless(desc.format) : ToDXGI(desc.format);

		D3D12_RESOURCE_DESC resourceDesc = {};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resourceDesc.Width = desc.width;
		resourceDesc.Height = desc.height;
		resourceDesc.DepthOrArraySize = (u16)desc.depth;
		resourceDesc.MipLevels = (u16)desc.mipLevels;
		resourceDesc.Format = resFormat;
		resourceDesc.SampleDesc = { 1, 0 };
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resourceDesc.Flags = flags;

		D3D12_CLEAR_VALUE clear = {};
		D3D12_CLEAR_VALUE* pClear = nullptr;
		if (isDS)
		{
			clear.Format = ToDXGI(desc.format);
			clear.DepthStencil.Depth = desc.clearDepth;
			clear.DepthStencil.Stencil = desc.clearStencil;
			pClear = &clear;
		}
		else if (isRT)
		{
			clear.Format = ToDXGI(desc.format);
			std::array<f32, 4> color =
			{
				desc.clearColor.r,
				desc.clearColor.g,
				desc.clearColor.b,
				desc.clearColor.a
			};
			std::memcpy(clear.Color, color.data(), sizeof(clear.Color));
			pClear = &clear;
		}

		D3D12MA::ALLOCATION_DESC allocDesc = {};
		allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

		GfxTextureHandle handle = TexturePoolAlloc();
		DX12Texture& texture = TexturePoolGet(handle);
		texture.format = resFormat;
		texture.width = desc.width; texture.height = desc.height;
		texture.state = D3D12_RESOURCE_STATE_COMMON;

		HRESULT hresult = context.allocator->CreateResource(&allocDesc, &resourceDesc, texture.state, pClear, &texture.memory, IID_PPV_ARGS(&texture.resource));
		CHECK_REASON(hresult, "Create Texture Resource");

		SetDebugName(texture.resource, desc.debugName);

		if (sampled)
		{
			texture.shaderViewIndex = BindlessAlloc();
			D3D12_SHADER_RESOURCE_VIEW_DESC shaderViewDesc = {};
			shaderViewDesc.Format = ToShaderViewFormat(desc.format);
			shaderViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

			if (is3D)
			{
				shaderViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
				shaderViewDesc.Texture3D.MipLevels = desc.mipLevels;
			}
			else if (isArray)
			{
				shaderViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
				shaderViewDesc.Texture2DArray.MipLevels = desc.mipLevels;
				shaderViewDesc.Texture2DArray.ArraySize = desc.depth;
			}
			else
			{
				shaderViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				shaderViewDesc.Texture2D.MipLevels = desc.mipLevels;
			}
			context.device->CreateShaderResourceView(texture.resource, &shaderViewDesc, BindlessCpu(texture.shaderViewIndex));
		}


		if (isRT && !is3D)
		{
			texture.targetViewIndex = context.nextTargetView++;
			D3D12_RENDER_TARGET_VIEW_DESC targetViewDesc = {};
			targetViewDesc.Format = ToDXGI(desc.format);
			targetViewDesc.ViewDimension = isArray ? D3D12_RTV_DIMENSION_TEXTURE2DARRAY : D3D12_RTV_DIMENSION_TEXTURE2D;
			if (isArray) targetViewDesc.Texture2DArray.ArraySize = desc.depth;
			auto cpuHandle = context.targetViewHeap->GetCPUDescriptorHandleForHeapStart();
			cpuHandle.ptr += usize(texture.targetViewIndex) * context.targetViewStride;
			context.device->CreateRenderTargetView(texture.resource, &targetViewDesc, cpuHandle);
		}

		if (isDS && !is3D)
		{
			texture.depthViewIndex = context.nextDepthView++;
			D3D12_DEPTH_STENCIL_VIEW_DESC depthViewDesc = {};
			depthViewDesc.Format = ToDXGI(desc.format);
			depthViewDesc.ViewDimension = isArray ? D3D12_DSV_DIMENSION_TEXTURE2DARRAY : D3D12_DSV_DIMENSION_TEXTURE2D;
			if (isArray) depthViewDesc.Texture2DArray.ArraySize = desc.depth;
			auto cpuHandle = context.depthViewHeap->GetCPUDescriptorHandleForHeapStart();
			cpuHandle.ptr += usize(texture.depthViewIndex) * context.depthViewStride;
			context.device->CreateDepthStencilView(texture.resource, &depthViewDesc, cpuHandle);
		}

		if (isUAV)
		{
			texture.accessViewIndex = BindlessAlloc();
			D3D12_UNORDERED_ACCESS_VIEW_DESC accessViewDesc = {};
			accessViewDesc.Format = ToDXGI(desc.format);

			if (is3D)
			{
				accessViewDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
				accessViewDesc.Texture3D.WSize = desc.depth;
				accessViewDesc.Texture3D.FirstWSlice = 0;
			}
			else if (isArray)
			{
				accessViewDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
				accessViewDesc.Texture2DArray.ArraySize = desc.depth;
			}
			else
			{
				accessViewDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
			}
			context.device->CreateUnorderedAccessView(texture.resource, nullptr, &accessViewDesc, BindlessCpu(texture.accessViewIndex));
		}

		return handle;
	}

	void GfxDevice::UploadTexture(GfxTextureHandle handle, const void* data, u32 mip, u32 slice)
	{
		Context& context = GfxContext();
		DX12Texture& texture = TexturePoolGet(handle);
		D3D12_RESOURCE_DESC resourceDesc = texture.resource->GetDesc();

		u32 subresource = mip + slice * resourceDesc.MipLevels;

		D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
		u32 numRows = 0; u64 rowSize = 0; u64 total = 0;
		context.device->GetCopyableFootprints(&resourceDesc, subresource, 1, 0, &footprint, &numRows, &rowSize, &total);

		ID3D12Resource* stagingResource = nullptr; D3D12MA::Allocation* stagingAlloc = nullptr;
		CreateStaging(total, &stagingResource, &stagingAlloc);

		u8* mappedData = nullptr; stagingResource->Map(0, nullptr, (void**)&mappedData);
		u64 slicePitch = (u64)footprint.Footprint.RowPitch * numRows;

		for (u32 depthIdx = 0; depthIdx < footprint.Footprint.Depth; depthIdx++)
		{
			for (u32 rowIdx = 0; rowIdx < numRows; rowIdx++)
				memcpy(mappedData + footprint.Offset + depthIdx * slicePitch + rowIdx * footprint.Footprint.RowPitch, (const u8*)data + ((u64)depthIdx * numRows + rowIdx) * rowSize, (usize)rowSize);
		}

		stagingResource->Unmap(0, nullptr);

		ImmediateSubmit([&](ID3D12GraphicsCommandList* list)
			{
				RawTransition(list, texture.resource, texture.state, D3D12_RESOURCE_STATE_COPY_DEST);

				D3D12_TEXTURE_COPY_LOCATION dstLoc = {};
				dstLoc.pResource = texture.resource;
				dstLoc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
				dstLoc.SubresourceIndex = subresource;

				D3D12_TEXTURE_COPY_LOCATION srcLoc = {};
				srcLoc.pResource = stagingResource;
				srcLoc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
				srcLoc.PlacedFootprint = footprint;

				list->CopyTextureRegion(&dstLoc, 0, 0, 0, &srcLoc, nullptr);
				RawTransition(list, texture.resource, D3D12_RESOURCE_STATE_COPY_DEST,
					D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			});

		texture.state = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

		stagingResource->Release(); stagingAlloc->Release();
	}

	void GfxDevice::DestroyTexture(GfxTextureHandle handle)
	{
		DX12Texture& texture = TexturePoolGet(handle);

		if (texture.shaderViewIndex != ~0u) 
			BindlessFree(texture.shaderViewIndex);
		if (texture.accessViewIndex != ~0u) 
			BindlessFree(texture.accessViewIndex);

		if (texture.resource && !texture.isBackbuffer) 
			texture.resource->Release();
		if (texture.memory) 
			texture.memory->Release();

		TexturePoolFree(handle);
	}

	u32 GfxDevice::BindlessShaderView(GfxTextureHandle handle) { return TexturePoolGet(handle).shaderViewIndex; }
	u32 GfxDevice::BindlessAccessView(GfxTextureHandle handle) { return TexturePoolGet(handle).accessViewIndex; }
}
