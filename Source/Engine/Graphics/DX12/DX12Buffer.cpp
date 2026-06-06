#include "DX12Backend.h"

#include <Engine/Log/Log.h>
#include <Engine/Graphics/RHI/GfxBuffer.h>

#define CHECK_REASON(hr, what) \
	if (FAILED(hr)) { ConsoleLog().Error("{}: {}", what, _com_error(hr).ErrorMessage()); }

namespace Horizon::DX12
{
	GfxBufferHandle BufferPoolAlloc()
	{
		Context& context = GfxContext();
		u32 index;
		if (!context.bufFreeList.empty()) { index = context.bufFreeList.back(); context.bufFreeList.pop_back(); }
		else { index = (u32)context.buffers.size(); context.buffers.push_back({}); context.bufGenerations.push_back(0); }
		return GfxBufferHandle::make(index, context.bufGenerations[index]);
	}

	DX12Buffer& BufferPoolGet(GfxBufferHandle handle) { return GfxContext().buffers[handle.index()]; }

	void BufferPoolFree(GfxBufferHandle handle)
	{
		Context& context = GfxContext();
		context.buffers[handle.index()] = {};
		context.bufGenerations[handle.index()]++;
		context.bufFreeList.push_back(handle.index());
	}
}

namespace Horizon
{
	using namespace DX12;

	GfxBufferHandle GfxDevice::CreateBuffer(const GfxBufferDesc& desc)
	{
		Context& context = GfxContext();

		D3D12_RESOURCE_DESC resourceDesc = {};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Width = u32(desc.size);
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		resourceDesc.SampleDesc = { 1, 0 };
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resourceDesc.Flags = (has(desc.usage, GfxBufferUsage::Storage) && desc.stride == 0)
			? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;

		D3D12MA::ALLOCATION_DESC allocDesc = {};
		allocDesc.HeapType = (desc.memory == GfxMemoryType::Upload) ? D3D12_HEAP_TYPE_UPLOAD
			: (desc.memory == GfxMemoryType::Readback) ? D3D12_HEAP_TYPE_READBACK
			: D3D12_HEAP_TYPE_DEFAULT;

		GfxBufferHandle handle = BufferPoolAlloc();
		DX12Buffer& buffer = BufferPoolGet(handle);
		buffer.size = desc.size;

		D3D12_RESOURCE_STATES initState =
			(desc.memory == GfxMemoryType::Upload) ? D3D12_RESOURCE_STATE_GENERIC_READ :
			(desc.memory == GfxMemoryType::Readback) ? D3D12_RESOURCE_STATE_COPY_DEST :
			D3D12_RESOURCE_STATE_COMMON;

		HRESULT hresult = context.allocator->CreateResource(&allocDesc, &resourceDesc, initState, nullptr,
			&buffer.memory, IID_PPV_ARGS(&buffer.resource));
		CHECK_REASON(hresult, "Create Buffer Resource");

		SetDebugName(buffer.resource, desc.debugName);

		buffer.gpuVA = buffer.resource->GetGPUVirtualAddress();

		if (desc.memory != GfxMemoryType::GPU)
			buffer.resource->Map(0, nullptr, &buffer.mapped);

		if (desc.stride > 0)
		{
			buffer.shaderViewIndex = BindlessAlloc();

			D3D12_SHADER_RESOURCE_VIEW_DESC shaderViewDesc = {};
			shaderViewDesc.Format = DXGI_FORMAT_UNKNOWN;
			shaderViewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			shaderViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			shaderViewDesc.Buffer.NumElements = u32(desc.size / desc.stride);
			shaderViewDesc.Buffer.StructureByteStride = desc.stride;

			context.device->CreateShaderResourceView(buffer.resource, &shaderViewDesc, BindlessCpu(buffer.shaderViewIndex));
		}
		else if (has(desc.usage, GfxBufferUsage::Storage))
		{
			buffer.shaderViewIndex = BindlessAlloc();

			D3D12_SHADER_RESOURCE_VIEW_DESC shaderViewDesc = {};
			shaderViewDesc.Format = DXGI_FORMAT_R32_TYPELESS;
			shaderViewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			shaderViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			shaderViewDesc.Buffer.NumElements = u32(desc.size / 4);
			shaderViewDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;

			context.device->CreateShaderResourceView(buffer.resource, &shaderViewDesc, BindlessCpu(buffer.shaderViewIndex));

			buffer.accessViewIndex = BindlessAlloc();

			D3D12_UNORDERED_ACCESS_VIEW_DESC accessViewDesc = {};
			accessViewDesc.Format = DXGI_FORMAT_R32_TYPELESS;
			accessViewDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			accessViewDesc.Buffer.NumElements = u32(desc.size / 4);
			accessViewDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;

			context.device->CreateUnorderedAccessView(buffer.resource, nullptr, &accessViewDesc, BindlessCpu(buffer.accessViewIndex));
		}

		return handle;
	}

	void GfxDevice::WriteBuffer(GfxBufferHandle handle, const void* data, usize size, usize offset)
	{
		DX12Buffer& buffer = BufferPoolGet(handle);
		memcpy((u8*)buffer.mapped + offset, data, size);
	}

	void GfxDevice::UploadBuffer(GfxBufferHandle handle, const void* data, usize size, usize offset)
	{
		DX12Buffer& dstBuffer = BufferPoolGet(handle);

		ID3D12Resource* stagingResource = nullptr; D3D12MA::Allocation* stagingAlloc = nullptr;
		CreateStaging(size, &stagingResource, &stagingAlloc);

		void* mapData = nullptr; stagingResource->Map(0, nullptr, &mapData);
		memcpy(mapData, data, size);
		stagingResource->Unmap(0, nullptr);

		ImmediateSubmit([&](ID3D12GraphicsCommandList* list)
			{
				RawTransition(list, dstBuffer.resource, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
				list->CopyBufferRegion(dstBuffer.resource, offset, stagingResource, 0, size);
				RawTransition(list, dstBuffer.resource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON);
			});

		stagingResource->Release();
		stagingAlloc->Release();
	}

	void GfxDevice::DestroyBuffer(GfxBufferHandle handle)
	{
		DX12Buffer& buffer = BufferPoolGet(handle);

		if (buffer.shaderViewIndex != ~0u)
			BindlessFree(buffer.shaderViewIndex);

		if (buffer.accessViewIndex != ~0u)
			BindlessFree(buffer.accessViewIndex);

		if (buffer.resource)
			buffer.resource->Release();

		if (buffer.memory)
			buffer.memory->Release();

		BufferPoolFree(handle);
	}

	u32 GfxDevice::BindlessShaderView(GfxBufferHandle handle) { return BufferPoolGet(handle).shaderViewIndex; }
	u32 GfxDevice::BindlessAccessView(GfxBufferHandle handle) { return BufferPoolGet(handle).accessViewIndex; }
}
