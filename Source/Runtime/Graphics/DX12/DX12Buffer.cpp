#include "DX12Context.h"

namespace Horizon
{
	GfxBufferHandle GfxDevice::CreateBuffer(const GfxBufferDesc& desc)
	{
		Context& context = DX12Context();

		usize size = desc.size;
		if (has(desc.usage, GfxBufferUsage::Constant))
			size = (size + 255) & ~usize(255);

		D3D12_RESOURCE_DESC resourceDesc = {};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Width = desc.size;
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

		D3D12_RESOURCE_STATES initState =
			(desc.memory == GfxMemoryType::Upload) ? D3D12_RESOURCE_STATE_GENERIC_READ :
			(desc.memory == GfxMemoryType::Readback) ? D3D12_RESOURCE_STATE_COPY_DEST :
			D3D12_RESOURCE_STATE_COMMON;

		DX12Buffer buffer = {};
		buffer.memory = desc.memory;

		HRESULT bResult = context.pAllocator->CreateResource(&allocDesc, &resourceDesc, initState,
			nullptr, &buffer.pMemory, IID_PPV_ARGS(&buffer.pResource));
		CHECK_REASON(bResult, "ID3D12Resource - CreateResource");

		if (FAILED(bResult))
			return GfxBufferHandle();

		buffer.sizeInBytes = desc.size;
		buffer.gpuVirtualAddress = buffer.pResource->GetGPUVirtualAddress();

		if (buffer.pMapped == nullptr && (desc.memory == GfxMemoryType::Upload || desc.memory == GfxMemoryType::Readback))
		{
			D3D12_RANGE noRead = { 0, 0 };
			buffer.pResource->Map(0, &noRead, &buffer.pMapped);
		}

		const b8 shaderRead = has(desc.usage, GfxBufferUsage::Vertex)
			|| has(desc.usage, GfxBufferUsage::Index)
			|| has(desc.usage, GfxBufferUsage::Storage);

		if (shaderRead)
			buffer.shaderViewIndex = Helpers::CreateBufferSRV(buffer, desc);

		if (has(desc.usage, GfxBufferUsage::Storage))    
			buffer.accessViewIndex = Helpers::CreateBufferUAV(buffer, desc);

		if (!desc.debugName.empty())
			buffer.pResource->SetPrivateData(WKPDID_D3DDebugObjectName, u32(desc.debugName.size()), desc.debugName.data());

		return context.bufferPool.Insert(std::move(buffer));
	}

	void GfxDevice::WriteBuffer(GfxBufferHandle handle, void* pData, usize sizeInBytes, usize offset)
	{
		Context& context = DX12Context();

		context.bufferPool.ResolveWrite(handle, [&](DX12Buffer& buffer)
			{
				assert(buffer.pMapped && "WriteBuffer is only being used for Upload/Readback");
				std::memcpy((u8*)buffer.pMapped + offset, pData, sizeInBytes);
			});
	}

	void GfxDevice::DestroyBuffer(GfxBufferHandle handle)
	{
		Context& context = DX12Context();

		context.bufferPool.ResolveWrite(handle, [&](DX12Buffer& buffer)
			{
				if (buffer.shaderViewIndex != ~0u)
					Helpers::FreeDescriptor(context.resourceHeap, buffer.shaderViewIndex);

				if (buffer.accessViewIndex != ~0u)
					Helpers::FreeDescriptor(context.resourceHeap, buffer.accessViewIndex);

				if (buffer.pMapped)
					buffer.pResource->Unmap(0, nullptr);

				if (buffer.pResource)
					buffer.pResource->Release();
				if (buffer.pMemory)
					buffer.pMemory->Release();
			});

		context.bufferPool.Remove(handle);
	}
}