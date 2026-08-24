#include "D3D12DescriptorHeap.h"

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>

#include <Runtime/D3D12/D3D12Buffer.h>
#include <Runtime/D3D12/D3D12Device.h>
#include <Runtime/D3D12/D3D12Texture.h>

namespace Horizon::RHI
{
	namespace
	{
		D3D12_SRV_DIMENSION ToSrvDimension(const GfxTextureDesc& desc)
		{
			switch (desc.type)
			{
			case GfxTextureType::Tex1D:
			{
				if (desc.arraySize > 1)
					return D3D12_SRV_DIMENSION_TEXTURE1DARRAY;

				return D3D12_SRV_DIMENSION_TEXTURE1D;
			}
			case GfxTextureType::Tex3D:
				return D3D12_SRV_DIMENSION_TEXTURE3D;
			default:
			{
				if (desc.isCube)
					return desc.arraySize > 1 ? D3D12_SRV_DIMENSION_TEXTURECUBEARRAY : D3D12_SRV_DIMENSION_TEXTURECUBE;

				if (desc.arraySize > 1)
					return D3D12_SRV_DIMENSION_TEXTURE2DARRAY;

				return D3D12_SRV_DIMENSION_TEXTURE2D;
			}
			}
		}

		D3D12_UAV_DIMENSION ToUavDimension(const GfxTextureDesc& desc)
		{
			switch (desc.type)
			{
			case GfxTextureType::Tex1D:
			{
				if (desc.arraySize > 1)
					return D3D12_UAV_DIMENSION_TEXTURE1DARRAY;

				return D3D12_UAV_DIMENSION_TEXTURE1D;
			}
			case GfxTextureType::Tex3D:
				return D3D12_UAV_DIMENSION_TEXTURE3D;
			default:
			{
				if (desc.arraySize > 1 || desc.isCube)
					return D3D12_UAV_DIMENSION_TEXTURE2DARRAY;

				return D3D12_UAV_DIMENSION_TEXTURE2D;
			}
			}
		}
	}

	D3D12DescriptorHeap::~D3D12DescriptorHeap()
	{
		if (m_heap)
			m_heap->Release();
	}

	u32 D3D12DescriptorHeap::Allocate()
	{
		if (!m_freeList.IsEmpty())
		{
			const u32 index = m_freeList.Back();

			m_freeList.PopBack();
			m_allocatedCount++;
			return index;
		}

		if (m_top >= m_desc.capacity)
		{
			Terminal::Error(StringOps::GetName(this), "Heap type {} is full, capacity {}", u32(m_desc.type), m_desc.capacity);
			return kInvalid32;
		}

		m_allocatedCount++;
		return m_top++;
	}

	void D3D12DescriptorHeap::Free(u32 index)
	{
		if (index == kInvalid32)
			return;

		m_freeList.PushBack(index);
		m_allocatedCount--;
	}

	b8 D3D12DescriptorHeap::ExpectType(GfxDescriptorHeapType type, const char* pWhat) const
	{
		if (m_desc.type == type)
			return true;

		Terminal::Error(StringOps::GetName(this), "{} needs heap type {}, this heap is {}",
			pWhat, u32(type), u32(m_desc.type));
		return false;
	}

	u32 D3D12DescriptorHeap::CreateShaderView(GfxTexture* pTexture)
	{
		if (!ExpectType(GfxDescriptorHeapType::Resource, "CreateShaderView"))
			return kInvalid32;

		auto* pD3DTexture = static_cast<D3D12Texture*>(pTexture);
		const GfxTextureDesc& texDesc = pD3DTexture->GetDesc();

		const u32 index = Allocate();

		if (index == kInvalid32)
			return kInvalid32;

		D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc = {};

		viewDesc.Format = Helpers::IsDepthFormat(pD3DTexture->m_format)
			? Helpers::ToDepthSRVFormat(pD3DTexture->m_format) : pD3DTexture->m_format;
		viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		viewDesc.ViewDimension = ToSrvDimension(texDesc);

		switch (viewDesc.ViewDimension)
		{
		case D3D12_SRV_DIMENSION_TEXTURE1D:
			viewDesc.Texture1D.MipLevels = texDesc.mipLevels;
			break;
		case D3D12_SRV_DIMENSION_TEXTURE1DARRAY:
			viewDesc.Texture1DArray.MipLevels = texDesc.mipLevels;
			viewDesc.Texture1DArray.ArraySize = texDesc.arraySize;
			break;
		case D3D12_SRV_DIMENSION_TEXTURE3D:
			viewDesc.Texture3D.MipLevels = texDesc.mipLevels;
			break;
		case D3D12_SRV_DIMENSION_TEXTURECUBE:
			viewDesc.TextureCube.MipLevels = texDesc.mipLevels;
			break;
		case D3D12_SRV_DIMENSION_TEXTURECUBEARRAY:
			viewDesc.TextureCubeArray.MipLevels = texDesc.mipLevels;
			viewDesc.TextureCubeArray.NumCubes = texDesc.arraySize;
			break;
		case D3D12_SRV_DIMENSION_TEXTURE2DARRAY:
			viewDesc.Texture2DArray.MipLevels = texDesc.mipLevels;
			viewDesc.Texture2DArray.ArraySize = texDesc.arraySize;
			break;
		default:
			viewDesc.Texture2D.MipLevels = texDesc.mipLevels;
			break;
		}

		auto* pDevice = static_cast<D3D12Device*>(m_ownerDevice);

		pDevice->Handle()->CreateShaderResourceView(pD3DTexture->m_resource, &viewDesc, CpuAt(index));
		pD3DTexture->m_shaderView = index;

		return index;
	}

	u32 D3D12DescriptorHeap::CreateStorageView(GfxTexture* pTexture)
	{
		if (!ExpectType(GfxDescriptorHeapType::Resource, "CreateStorageView"))
			return kInvalid32;

		auto* pD3DTexture = static_cast<D3D12Texture*>(pTexture);
		const GfxTextureDesc& texDesc = pD3DTexture->GetDesc();

		const u32 index = Allocate();

		if (index == kInvalid32)
			return kInvalid32;

		D3D12_UNORDERED_ACCESS_VIEW_DESC viewDesc = {};

		viewDesc.Format = pD3DTexture->m_format;
		viewDesc.ViewDimension = ToUavDimension(texDesc);

		switch (viewDesc.ViewDimension)
		{
		case D3D12_UAV_DIMENSION_TEXTURE1DARRAY:
			viewDesc.Texture1DArray.ArraySize = texDesc.arraySize;
			break;
		case D3D12_UAV_DIMENSION_TEXTURE2DARRAY:
			viewDesc.Texture2DArray.ArraySize = texDesc.isCube ? texDesc.arraySize * 6 : texDesc.arraySize;
			break;
		case D3D12_UAV_DIMENSION_TEXTURE3D:
			viewDesc.Texture3D.WSize = texDesc.depth;
			break;
		default:
			break;
		}

		auto* pDevice = static_cast<D3D12Device*>(m_ownerDevice);

		pDevice->Handle()->CreateUnorderedAccessView(pD3DTexture->m_resource, nullptr, &viewDesc, CpuAt(index));
		pD3DTexture->m_storageView = index;

		return index;
	}

	u32 D3D12DescriptorHeap::CreateRenderTargetView(GfxTexture* pTexture)
	{
		if (!ExpectType(GfxDescriptorHeapType::Color, "CreateRenderTargetView"))
			return kInvalid32;

		auto* pD3DTexture = static_cast<D3D12Texture*>(pTexture);
		const u32 index = Allocate();

		if (index == kInvalid32)
			return kInvalid32;

		auto* pDevice = static_cast<D3D12Device*>(m_ownerDevice);

		pDevice->Handle()->CreateRenderTargetView(pD3DTexture->m_resource, nullptr, CpuAt(index));

		pD3DTexture->m_renderTargetView = index;
		pD3DTexture->m_renderTargetHandle = CpuAt(index);

		return index;
	}

	u32 D3D12DescriptorHeap::CreateDepthStencilView(GfxTexture* pTexture)
	{
		if (!ExpectType(GfxDescriptorHeapType::Depth, "CreateDepthStencilView"))
			return kInvalid32;

		auto* pD3DTexture = static_cast<D3D12Texture*>(pTexture);
		const GfxTextureDesc& texDesc = pD3DTexture->GetDesc();

		const u32 index = Allocate();

		if (index == kInvalid32)
			return kInvalid32;

		D3D12_DEPTH_STENCIL_VIEW_DESC viewDesc = {};

		viewDesc.Format = pD3DTexture->m_format;
		viewDesc.ViewDimension = texDesc.arraySize > 1
			? D3D12_DSV_DIMENSION_TEXTURE2DARRAY : D3D12_DSV_DIMENSION_TEXTURE2D;

		if (viewDesc.ViewDimension == D3D12_DSV_DIMENSION_TEXTURE2DARRAY)
			viewDesc.Texture2DArray.ArraySize = texDesc.arraySize;

		auto* pDevice = static_cast<D3D12Device*>(m_ownerDevice);

		pDevice->Handle()->CreateDepthStencilView(pD3DTexture->m_resource, &viewDesc, CpuAt(index));

		pD3DTexture->m_depthStencilView = index;
		pD3DTexture->m_depthStencilHandle = CpuAt(index);

		return index;
	}

	u32 D3D12DescriptorHeap::CreateShaderView(GfxBuffer* pBuffer)
	{
		if (!ExpectType(GfxDescriptorHeapType::Resource, "CreateShaderView"))
			return kInvalid32;

		auto* pD3DBuffer = static_cast<D3D12Buffer*>(pBuffer);
		const GfxBufferDesc& bufDesc = pD3DBuffer->GetDesc();

		const u32 index = Allocate();

		if (index == kInvalid32)
			return kInvalid32;

		D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc = {};

		viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		viewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;

		if (bufDesc.stride > 0)
		{
			viewDesc.Format = DXGI_FORMAT_UNKNOWN;
			viewDesc.Buffer.NumElements = u32(bufDesc.size / bufDesc.stride);
			viewDesc.Buffer.StructureByteStride = bufDesc.stride;
		}
		else
		{
			viewDesc.Format = DXGI_FORMAT_R32_TYPELESS;
			viewDesc.Buffer.NumElements = u32(bufDesc.size / 4);
			viewDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
		}

		auto* pDevice = static_cast<D3D12Device*>(m_ownerDevice);

		pDevice->Handle()->CreateShaderResourceView(pD3DBuffer->m_resource, &viewDesc, CpuAt(index));
		pD3DBuffer->m_shaderView = index;

		return index;
	}

	u32 D3D12DescriptorHeap::CreateStorageView(GfxBuffer* pBuffer)
	{
		if (!ExpectType(GfxDescriptorHeapType::Resource, "CreateStorageView"))
			return kInvalid32;

		auto* pD3DBuffer = static_cast<D3D12Buffer*>(pBuffer);
		const GfxBufferDesc& bufDesc = pD3DBuffer->GetDesc();

		const u32 index = Allocate();

		if (index == kInvalid32)
			return kInvalid32;

		D3D12_UNORDERED_ACCESS_VIEW_DESC viewDesc = {};

		viewDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;

		if (bufDesc.stride > 0)
		{
			viewDesc.Format = DXGI_FORMAT_UNKNOWN;
			viewDesc.Buffer.NumElements = u32(bufDesc.size / bufDesc.stride);
			viewDesc.Buffer.StructureByteStride = bufDesc.stride;
		}
		else
		{
			viewDesc.Format = DXGI_FORMAT_R32_TYPELESS;
			viewDesc.Buffer.NumElements = u32(bufDesc.size / 4);
			viewDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
		}

		auto* pDevice = static_cast<D3D12Device*>(m_ownerDevice);

		pDevice->Handle()->CreateUnorderedAccessView(pD3DBuffer->m_resource, nullptr, &viewDesc, CpuAt(index));
		pD3DBuffer->m_storageView = index;

		return index;
	}
}
