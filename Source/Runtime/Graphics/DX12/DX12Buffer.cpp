#include "DX12Buffer.h"

#include <Runtime/Graphics/RHI/GfxBuffer.h>
#include <Runtime/Graphics/RHI/GfxDevice.h>

#include <Runtime/Graphics/DX12/DX12Device.h>

#define CHECK_HR(hr, what) \
	if(FAILED(hr)) { Horizon::MainLog::Error("{}: {}", what, _com_error(hr).ErrorMessage()); exit(-1); }

#define CHECK_REASON(hr, what) \
	if (FAILED(hr)) { Horizon::MainLog::Error("{}: {}", what, _com_error(hr).ErrorMessage()); }

namespace Horizon
{
	GfxBuffer::GfxBuffer(const GfxBufferDesc& desc, GfxDevice* pDevice) : GfxObject(pDevice), m_desc(desc)
	{
		DX12Device* deviceNative = static_cast<DX12Device*>(pDevice->GetNative());

		DX12Buffer* pNative = new DX12Buffer();

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

		HRESULT hresult = deviceNative->allocator->CreateResource(&allocDesc, &resourceDesc, initState, nullptr,
			&pNative->memory, IID_PPV_ARGS(&pNative->resource));
		CHECK_REASON(hresult, "Create Buffer Resource");

		if (desc.memory != GfxMemoryType::GPU)
			pNative->resource->Map(0, nullptr, &m_mapped);

		m_gpuAddress = pNative->resource->GetGPUVirtualAddress();

		if (desc.stride > 0)
		{
			m_shaderView = deviceNative->bindless.Allocate();

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Buffer.NumElements = u32(desc.size / desc.stride);
			srvDesc.Buffer.StructureByteStride = desc.stride;

			deviceNative->device->CreateShaderResourceView(pNative->resource, &srvDesc, deviceNative->bindless.GetCPUHandle(m_shaderView));
		}
		else if (has(desc.usage, GfxBufferUsage::Storage))
		{
			m_shaderView = deviceNative->bindless.Allocate();

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Buffer.NumElements = u32(desc.size / 4);
			srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;

			deviceNative->device->CreateShaderResourceView(pNative->resource, &srvDesc, deviceNative->bindless.GetCPUHandle(m_shaderView));

			m_accessView = deviceNative->bindless.Allocate();

			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			uavDesc.Buffer.NumElements = u32(desc.size / 4);
			uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;

			deviceNative->device->CreateUnorderedAccessView(pNative->resource, nullptr, &uavDesc, deviceNative->bindless.GetCPUHandle(m_accessView));
		}

		m_native = pNative;
	}

	GfxBuffer::~GfxBuffer()
	{
		// TODO: inFrame destroy->fence-deferred
		DX12Device* deviceNative = static_cast<DX12Device*>(m_device->GetNative());
		DX12Buffer* pNative = static_cast<DX12Buffer*>(m_native);

		if (m_shaderView != ~0u)
			deviceNative->bindless.Free(m_shaderView);
		if (m_accessView != ~0u)
			deviceNative->bindless.Free(m_accessView);

		if (pNative)
		{
			if (pNative->resource)
				pNative->resource->Release();
			if (pNative->memory)
				pNative->memory->Release();
			delete pNative;
		}
	}

	void GfxBuffer::Write(const void* pData, usize count, usize offset) const
	{
		std::memcpy((u8*)m_mapped + offset, pData, count * m_desc.stride);
	}
}