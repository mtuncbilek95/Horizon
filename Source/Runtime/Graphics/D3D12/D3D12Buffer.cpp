#include "D3D12Backend.h"

#include <Runtime/Graphics/GfxBackend.h>

namespace Horizon
{
	GfxBuffer* Gfx::CreateGfxBuffer(GfxDevice* pContext, const GfxBufferDesc& desc)
	{
		usize size = desc.size;
		if (has(desc.usage, GfxBufferUsage::Constant))
			size = (size + 255) & ~usize(255);

		D3D12_RESOURCE_DESC resourceDesc = {};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Width = size;
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		resourceDesc.SampleDesc = { 1, 0 };
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resourceDesc.Flags = has(desc.usage, GfxBufferUsage::Storage)
			? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;

		D3D12MA::ALLOCATION_DESC allocDesc = {};
		allocDesc.HeapType =
			desc.memory == GfxMemoryType::HostVisible ? D3D12_HEAP_TYPE_UPLOAD :
			desc.memory == GfxMemoryType::CPU ? D3D12_HEAP_TYPE_READBACK :
			D3D12_HEAP_TYPE_DEFAULT;

		const D3D12_RESOURCE_STATES initialState =
			desc.memory == GfxMemoryType::HostVisible ? D3D12_RESOURCE_STATE_GENERIC_READ :
			desc.memory == GfxMemoryType::CPU ? D3D12_RESOURCE_STATE_COPY_DEST :
			D3D12_RESOURCE_STATE_COMMON;

		GfxBuffer* pBuffer = new GfxBuffer();
		HRESULT bResult = pContext->pAllocator->CreateResource(&allocDesc, &resourceDesc,
			initialState, nullptr, &pBuffer->pMemory, IID_PPV_ARGS(&pBuffer->pResource));
		CHECK_REASON(bResult, "ID3D12Resource - CreateResource (Buffer)");

		if (FAILED(bResult))
		{
			delete pBuffer;
			return nullptr;
		}

		pBuffer->memory = desc.memory;
		pBuffer->sizeInBytes = size;
		pBuffer->stride = desc.stride;
		pBuffer->gpuAddress = pBuffer->pResource->GetGPUVirtualAddress();

		if (desc.memory != GfxMemoryType::GPU)
		{
			D3D12_RANGE noRead = { 0, 0 };
			pBuffer->pResource->Map(0, &noRead, &pBuffer->pMapped);
		}

		return pBuffer;
	}

	void Gfx::WriteGfxBuffer(GfxBuffer* pBuffer, const void* pData, usize sizeInBytes, usize offset)
	{
		assert(pBuffer->pMapped && "WriteGfxBuffer requires a CPU-visible (HostVisible/CPU) buffer");
		std::memcpy((u8*)pBuffer->pMapped + offset, pData, sizeInBytes);
	}

	u32 Gfx::CreateBufferSRV(GfxDevice* pContext, GfxDescriptorHeap* pHeap, GfxBuffer* pBuffer)
	{
		u32 index = Helpers::AllocateDescriptor(pHeap);

		D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
		viewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		if (pBuffer->stride > 0)
		{
			viewDesc.Format = DXGI_FORMAT_UNKNOWN;
			viewDesc.Buffer.NumElements = u32(pBuffer->sizeInBytes / pBuffer->stride);
			viewDesc.Buffer.StructureByteStride = pBuffer->stride;
		}
		else
		{
			viewDesc.Format = DXGI_FORMAT_R32_TYPELESS;
			viewDesc.Buffer.NumElements = u32(pBuffer->sizeInBytes / 4);
			viewDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
		}

		pContext->pDevice->CreateShaderResourceView(pBuffer->pResource, &viewDesc,
			Helpers::CpuAt(pHeap, index));

		pBuffer->shaderViewIndex = index;
		return index;
	}

	u32 Gfx::CreateBufferUAV(GfxDevice* pContext, GfxDescriptorHeap* pHeap, GfxBuffer* pBuffer)
	{
		u32 index = Helpers::AllocateDescriptor(pHeap);

		D3D12_UNORDERED_ACCESS_VIEW_DESC viewDesc = {};
		viewDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;

		if (pBuffer->stride > 0)
		{
			viewDesc.Format = DXGI_FORMAT_UNKNOWN;
			viewDesc.Buffer.NumElements = u32(pBuffer->sizeInBytes / pBuffer->stride);
			viewDesc.Buffer.StructureByteStride = pBuffer->stride;
		}
		else
		{
			viewDesc.Format = DXGI_FORMAT_R32_TYPELESS;
			viewDesc.Buffer.NumElements = u32(pBuffer->sizeInBytes / 4);
			viewDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
		}

		pContext->pDevice->CreateUnorderedAccessView(pBuffer->pResource, nullptr, &viewDesc,
			Helpers::CpuAt(pHeap, index));

		pBuffer->accessViewIndex = index;
		return index;
	}

	u32 Gfx::GetBufferShaderView(const GfxBuffer* pBuffer)
	{
		return pBuffer->shaderViewIndex;
	}

	u32 Gfx::GetBufferAccessView(const GfxBuffer* pBuffer)
	{
		return pBuffer->accessViewIndex;
	}

	void Gfx::DestroyGfxBuffer(GfxBuffer* bufHandl)
	{
		if (bufHandl->pMapped)
			bufHandl->pResource->Unmap(0, nullptr);

		if (bufHandl->pResource)
			bufHandl->pResource->Release();
		if (bufHandl->pMemory)
			bufHandl->pMemory->Release();

		delete bufHandl;
	}
}