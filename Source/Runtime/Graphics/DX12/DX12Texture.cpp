#include "DX12Texture.h"

#include <Runtime/Graphics/RHI/GfxTexture.h>
#include <Runtime/Graphics/RHI/GfxDevice.h>

#include <Runtime/Graphics/DX12/DX12Device.h>
#include <Runtime/Graphics/DX12/DX12Utils.h>

#define CHECK_REASON(hr, what) \
	if (FAILED(hr)) { Horizon::MainLog::Error("{}: {}", what, _com_error(hr).ErrorMessage()); }

namespace Horizon
{
	GfxTexture::GfxTexture(const GfxTextureDesc& desc, GfxDevice* pDevice) : GfxObject(pDevice), m_desc(desc)
	{
		DX12Device* deviceNative = static_cast<DX12Device*>(pDevice->GetNative());

		DX12Texture* pNative = new DX12Texture();

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
		resourceDesc.Dimension = is3D ? D3D12_RESOURCE_DIMENSION_TEXTURE3D : D3D12_RESOURCE_DIMENSION_TEXTURE2D;
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
			clear.Color[0] = desc.clearColor.r;
			clear.Color[1] = desc.clearColor.g;
			clear.Color[2] = desc.clearColor.b;
			clear.Color[3] = desc.clearColor.a;
			pClear = &clear;
		}

		D3D12MA::ALLOCATION_DESC allocDesc = {};
		allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

		pNative->state = D3D12_RESOURCE_STATE_COMMON;
		pNative->format = resFormat;

		HRESULT hresult = deviceNative->allocator->CreateResource(&allocDesc, &resourceDesc, pNative->state, pClear,
			&pNative->memory, IID_PPV_ARGS(&pNative->resource));
		CHECK_REASON(hresult, "Create Texture Resource");

		if (sampled)
		{
			m_shaderView = deviceNative->bindless.Allocate();

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = ToShaderViewFormat(desc.format);
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

			if (is3D)
			{
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
				srvDesc.Texture3D.MipLevels = desc.mipLevels;
			}
			else if (isArray)
			{
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
				srvDesc.Texture2DArray.MipLevels = desc.mipLevels;
				srvDesc.Texture2DArray.ArraySize = desc.depth;
			}
			else
			{
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				srvDesc.Texture2D.MipLevels = desc.mipLevels;
			}

			deviceNative->device->CreateShaderResourceView(pNative->resource, &srvDesc, deviceNative->bindless.GetCPUHandle(m_shaderView));
		}

		if (isRT && !is3D)
		{
			m_targetView = deviceNative->rtv.Allocate();

			D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
			rtvDesc.Format = ToDXGI(desc.format);
			rtvDesc.ViewDimension = isArray ? D3D12_RTV_DIMENSION_TEXTURE2DARRAY : D3D12_RTV_DIMENSION_TEXTURE2D;
			if (isArray)
				rtvDesc.Texture2DArray.ArraySize = desc.depth;

			deviceNative->device->CreateRenderTargetView(pNative->resource, &rtvDesc, deviceNative->rtv.GetCPUHandle(m_targetView));
		}

		if (isDS && !is3D)
		{
			m_depthView = deviceNative->dsv.Allocate();

			D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
			dsvDesc.Format = ToDXGI(desc.format);
			dsvDesc.ViewDimension = isArray ? D3D12_DSV_DIMENSION_TEXTURE2DARRAY : D3D12_DSV_DIMENSION_TEXTURE2D;
			if (isArray)
				dsvDesc.Texture2DArray.ArraySize = desc.depth;

			deviceNative->device->CreateDepthStencilView(pNative->resource, &dsvDesc, deviceNative->dsv.GetCPUHandle(m_depthView));
		}

		if (isUAV)
		{
			m_accessView = deviceNative->bindless.Allocate();

			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			uavDesc.Format = ToDXGI(desc.format);

			if (is3D)
			{
				uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
				uavDesc.Texture3D.WSize = desc.depth;
				uavDesc.Texture3D.FirstWSlice = 0;
			}
			else if (isArray)
			{
				uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
				uavDesc.Texture2DArray.ArraySize = desc.depth;
			}
			else
			{
				uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
			}

			deviceNative->device->CreateUnorderedAccessView(pNative->resource, nullptr, &uavDesc, deviceNative->bindless.GetCPUHandle(m_accessView));
		}

		m_native = pNative;
	}

	GfxTexture::GfxTexture(GfxDevice* pDevice, GfxHandle backbufferResource, const GfxTextureDesc& desc)
		: GfxObject(pDevice), m_desc(desc)
	{
		DX12Device* deviceNative = static_cast<DX12Device*>(pDevice->GetNative());

		DX12Texture* pNative = new DX12Texture();
		pNative->resource = static_cast<ID3D12Resource*>(backbufferResource);
		pNative->isBackbuffer = true;
		pNative->format = ToDXGI(desc.format);
		pNative->state = D3D12_RESOURCE_STATE_PRESENT;

		m_targetView = deviceNative->rtv.Allocate();
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = ToDXGI(desc.format);
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		deviceNative->device->CreateRenderTargetView(pNative->resource, &rtvDesc, deviceNative->rtv.GetCPUHandle(m_targetView));

		m_native = pNative;
	}

	GfxTexture::~GfxTexture()
	{
		DX12Device* deviceNative = static_cast<DX12Device*>(m_device->GetNative());
		DX12Texture* pNative = static_cast<DX12Texture*>(m_native);

		// Backbuffer: swapchain sahibi, resize/destroy WaitIdle sonrası çağrılır -> immediate (deferred değil)
		if (pNative && pNative->isBackbuffer)
		{
			if (m_targetView != ~0u)
				deviceNative->rtv.Free(m_targetView);
			if (pNative->resource)
				pNative->resource->Release();   // GetBuffer'ın verdiği referans
			delete pNative;
			return;
		}

		u32 shaderView = m_shaderView;
		u32 accessView = m_accessView;
		u32 targetView = m_targetView;
		u32 depthView = m_depthView;

		m_device->DeferDestroy([deviceNative, pNative, shaderView, accessView, targetView, depthView]
			{
				if (shaderView != ~0u)
					deviceNative->bindless.Free(shaderView);
				if (accessView != ~0u)
					deviceNative->bindless.Free(accessView);
				if (targetView != ~0u)
					deviceNative->rtv.Free(targetView);
				if (depthView != ~0u)
					deviceNative->dsv.Free(depthView);

				if (pNative)
				{
					if (pNative->resource && !pNative->isBackbuffer)
						pNative->resource->Release();
					if (pNative->memory)
						pNative->memory->Release();
					delete pNative;
				}
			});
	}
}
