#include "D3D12DescriptorHeap.h"

#include <Runtime/D3D12/Buffer/D3D12Buffer.h>
#include <Runtime/D3D12/Sampler/D3D12Sampler.h>
#include <Runtime/D3D12/Texture/D3D12Texture.h>
#include <Runtime/D3D12/Texture/D3D12TextureView.h>
#include <Runtime/Containers/ScopedLock.h>
#include <Runtime/Containers/StringHelper.h>
#include <Runtime/Log/Terminal.h>

#include <cstring>

namespace Horizon::RHI
{
	D3D12DescriptorHeap::~D3D12DescriptorHeap()
	{
		if (m_heap)
			m_heap->Release();
	}

	void D3D12DescriptorHeap::SetDebugName(const char* pName)
	{
		if (!m_heap || !pName)
			return;

		m_heap->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(std::strlen(pName)), pName);
	}

	u32 D3D12DescriptorHeap::Allocate()
	{
		return AllocateRange(1);
	}

	void D3D12DescriptorHeap::Free(u32 index)
	{
		if (index == kInvalid32)
			return;

		FreeRange(index, 1);
	}

	void D3D12DescriptorHeap::Write(u32 index, DescriptorType type, TextureView* pView)
	{
		if (m_type != DescriptorHeapType::Resource || index >= m_capacity)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Texture write is invalid, index: {} capacity: {}", index, m_capacity);
			return;
		}

		D3D12TextureView* pD3DView = static_cast<D3D12TextureView*>(pView);
		D3D12Texture* pTexture = static_cast<D3D12Texture*>(pView->GetTexture());
		D3D12_CPU_DESCRIPTOR_HANDLE handle = CpuAt(index);

		if (type == DescriptorType::Texture)
		{
			if (!HasFlag(pTexture->GetUsage(), TextureUsage::Sampled))
			{
				Terminal::Error(StringHelper::GetTypeName(this), "Index {} needs a Sampled texture", index);
				return;
			}

			m_device->CreateShaderResourceView(pTexture->Handle(), &pD3DView->ShaderResourceDesc(), handle);
		}
		else if (type == DescriptorType::RWTexture)
		{
			if (!HasFlag(pTexture->GetUsage(), TextureUsage::Storage))
			{
				Terminal::Error(StringHelper::GetTypeName(this), "Index {} needs a Storage texture", index);
				return;
			}

			m_device->CreateUnorderedAccessView(pTexture->Handle(), nullptr, &pD3DView->UnorderedAccessDesc(), handle);
		}
		else
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Index {} is written with a non-texture type", index);
		}
	}

	void D3D12DescriptorHeap::Write(u32 index, DescriptorType type, Buffer* pBuffer, u64 offset, u64 size, u32 stride)
	{
		if (m_type != DescriptorHeapType::Resource || index >= m_capacity)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Buffer write is invalid, index: {} capacity: {}", index, m_capacity);
			return;
		}

		D3D12Buffer* pD3DBuffer = static_cast<D3D12Buffer*>(pBuffer);
		D3D12_CPU_DESCRIPTOR_HANDLE handle = CpuAt(index);

		if (type == DescriptorType::ConstantBuffer)
		{
			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
			cbvDesc.BufferLocation = pD3DBuffer->GetGpuAddress() + offset;
			cbvDesc.SizeInBytes = static_cast<UINT>((size + D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1) & ~u64(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1));

			m_device->CreateConstantBufferView(&cbvDesc, handle);
		}
		else if (type == DescriptorType::Buffer)
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

			if (stride == 0)
			{
				srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
				srvDesc.Buffer.FirstElement = offset / 4;
				srvDesc.Buffer.NumElements = static_cast<UINT>(size / 4);
				srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
			}
			else
			{
				srvDesc.Format = DXGI_FORMAT_UNKNOWN;
				srvDesc.Buffer.FirstElement = offset / stride;
				srvDesc.Buffer.NumElements = static_cast<UINT>(size / stride);
				srvDesc.Buffer.StructureByteStride = stride;
				srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
			}

			m_device->CreateShaderResourceView(pD3DBuffer->Handle(), &srvDesc, handle);
		}
		else if (type == DescriptorType::RWBuffer)
		{
			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;

			if (stride == 0)
			{
				uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
				uavDesc.Buffer.FirstElement = offset / 4;
				uavDesc.Buffer.NumElements = static_cast<UINT>(size / 4);
				uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
			}
			else
			{
				uavDesc.Format = DXGI_FORMAT_UNKNOWN;
				uavDesc.Buffer.FirstElement = offset / stride;
				uavDesc.Buffer.NumElements = static_cast<UINT>(size / stride);
				uavDesc.Buffer.StructureByteStride = stride;
				uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
			}

			m_device->CreateUnorderedAccessView(pD3DBuffer->Handle(), nullptr, &uavDesc, handle);
		}
		else
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Index {} is written with a non-buffer type", index);
		}
	}

	void D3D12DescriptorHeap::Write(u32 index, Sampler* pSampler)
	{
		if (m_type != DescriptorHeapType::Sampler || index >= m_capacity)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Sampler write is invalid, index: {} capacity: {}", index, m_capacity);
			return;
		}

		m_device->CreateSampler(&static_cast<D3D12Sampler*>(pSampler)->NativeDesc(), CpuAt(index));
	}

	u32 D3D12DescriptorHeap::AllocateRange(u32 count)
	{
		ScopedLock lock(m_lock);

		for (usize i = 0; i < m_freeBlocks.GetCount(); i++)
		{
			Block& block = m_freeBlocks[i];
			if (block.count < count)
				continue;

			u32 offset = block.offset;
			block.offset += count;
			block.count -= count;

			if (block.count == 0)
				m_freeBlocks.RemoveAt(i);

			return offset;
		}

		Terminal::Error(StringHelper::GetTypeName(this), "Heap has no room for {} descriptors, capacity: {}", count, m_capacity);
		return kInvalid32;
	}

	void D3D12DescriptorHeap::FreeRange(u32 offset, u32 count)
	{
		if (count == 0)
			return;

		ScopedLock lock(m_lock);

		usize index = 0;
		while (index < m_freeBlocks.GetCount() && m_freeBlocks[index].offset < offset)
			index++;

		m_freeBlocks.PushAt(index, Block{ offset, count });

		if (index + 1 < m_freeBlocks.GetCount())
		{
			const Block& next = m_freeBlocks[index + 1];
			if (offset + count == next.offset)
			{
				m_freeBlocks[index].count += next.count;
				m_freeBlocks.RemoveAt(index + 1);
			}
		}

		if (index > 0)
		{
			Block& prev = m_freeBlocks[index - 1];
			if (prev.offset + prev.count == m_freeBlocks[index].offset)
			{
				prev.count += m_freeBlocks[index].count;
				m_freeBlocks.RemoveAt(index);
			}
		}
	}
}