#include "D3D12Device.h"

#include <Runtime/D3D12/Buffer/D3D12Buffer.h>
#include <Runtime/D3D12/Command/D3D12CommandList.h>
#include <Runtime/D3D12/Command/D3D12CommandPool.h>
#include <Runtime/D3D12/Helpers/D3D12Helpers.h>
#include <Runtime/D3D12/Descriptor/D3D12DescriptorHeap.h>
#include <Runtime/D3D12/Descriptor/D3D12DescriptorTable.h>
#include <Runtime/D3D12/Fence/D3D12Fence.h>
#include <Runtime/D3D12/Pipeline/D3D12ComputePipeline.h>
#include <Runtime/D3D12/Pipeline/D3D12GraphicsPipeline.h>
#include <Runtime/D3D12/Pipeline/D3D12PipelineLayout.h>
#include <Runtime/D3D12/Pipeline/D3D12PipelineStream.h>
#include <Runtime/D3D12/Sampler/D3D12Sampler.h>
#include <Runtime/D3D12/Shader/D3D12ShaderModule.h>
#include <Runtime/D3D12/Swapchain/D3D12Swapchain.h>
#include <Runtime/D3D12/Texture/D3D12Texture.h>
#include <Runtime/D3D12/Texture/D3D12TextureView.h>
#include <Runtime/Containers/StringHelper.h>
#include <Runtime/Log/Terminal.h>

#include <imgui.h>
#include <backends/imgui_impl_dx12.h>

namespace Horizon::RHI
{
	namespace
	{
		std::string ResultToInfo(HRESULT result)
		{
			std::string info = std::format("0x{:08X}", static_cast<u32>(result));

			c8* pMessage = nullptr;
			DWORD length = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				nullptr, static_cast<DWORD>(result), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPSTR>(&pMessage), 0, nullptr);

			if (length == 0 || !pMessage)
				return info;

			std::string message(pMessage, length);
			LocalFree(pMessage);

			while (!message.empty() && (message.back() == '\n' || message.back() == '\r' || message.back() == ' '))
				message.pop_back();

			return info + " " + message;
		}

		void ImGuiAlloc(ImGui_ImplDX12_InitInfo* pInfo, D3D12_CPU_DESCRIPTOR_HANDLE* pOutCpu, D3D12_GPU_DESCRIPTOR_HANDLE* pOutGpu)
		{
			D3D12DescriptorHeap* pHeap = static_cast<D3D12DescriptorHeap*>(pInfo->UserData);

			u32 index = pHeap->Allocate();
			if (index == kInvalid32)
			{
				Terminal::Error("D3D12Device", "ImGui could not take a descriptor, resource heap is full");
				*pOutCpu = {};
				*pOutGpu = {};
				return;
			}

			*pOutCpu = pHeap->CpuAt(index);
			*pOutGpu = pHeap->GpuAt(index);
		}

		void ImGuiFree(ImGui_ImplDX12_InitInfo* pInfo, D3D12_CPU_DESCRIPTOR_HANDLE cpu, D3D12_GPU_DESCRIPTOR_HANDLE gpu)
		{
			D3D12DescriptorHeap* pHeap = static_cast<D3D12DescriptorHeap*>(pInfo->UserData);
			pHeap->Free(pHeap->IndexOf(cpu));
		}
	}

	UniquePtr<Device> CreateRHIDevice(DeviceDesc& desc)
	{
		UniquePtr<D3D12Device> pDevice = MakeUnique<D3D12Device>();
		if (pDevice->Initialize(desc))
			return pDevice;

		return nullptr;
	}

	D3D12Device::D3D12Device()
	{
	}

	D3D12Device::~D3D12Device()
	{
		CloseAll();
	}

	b8 D3D12Device::Initialize(const DeviceDesc& desc)
	{
		// Handle debug
		u32 factoryFlags = 0;
		if (desc.enableDebug && SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&m_debug))))
		{
			m_debug->EnableDebugLayer();
			factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;

			if (desc.enableGpuValidation)
				m_debug->SetEnableGPUBasedValidation(TRUE);

			if (desc.synchronizedCommandValidation)
				m_debug->SetEnableGPUBasedValidation(TRUE);
		}

		// Create the instance
		HRESULT hr = CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&m_factory));
		if (FAILED(hr))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "CreateDXGIFactory2:{}", ResultToInfo(hr));
			CloseAll();
			return false;
		}

		// Get the best adapter
		// TODO: Adapter generation can be a thing in future
		hr = m_factory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
			IID_PPV_ARGS(&m_adapter));
		if (FAILED(hr))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "EnumAdapterByGpuPreference:{}", ResultToInfo(hr));
			CloseAll();
			return false;
		}

		// Create the base device
		ID3D12Device* pBaseDevice = nullptr;
		hr = D3D12CreateDevice(m_adapter, D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&pBaseDevice));
		if (FAILED(hr))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "D3D12CreateDevice:{}", ResultToInfo(hr));
			CloseAll();
			return false;
		}

		// Handle the query interface
		// Self-note: Why the fuck its named as QueryInterface? Isn't it just a fancy casting?
		hr = pBaseDevice->QueryInterface(IID_PPV_ARGS(&m_device));
		if (FAILED(hr))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "QueryInterface To ID3DDevice10:{}", ResultToInfo(hr));
			CloseAll();
			return false;
		}
		pBaseDevice->Release();

		// Create the allocator
		D3D12MA::ALLOCATOR_DESC allocDesc = {};

		allocDesc.pDevice = m_device;
		allocDesc.pAdapter = m_adapter;
		hr = D3D12MA::CreateAllocator(&allocDesc, &m_allocator);
		if (FAILED(hr))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "CreateAllocator:{}", ResultToInfo(hr));
			CloseAll();
			return false;
		}

		hr = m_rtvPool.Initialize(m_device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1024);
		if (FAILED(hr))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "RTV Pool:{}", ResultToInfo(hr));
			CloseAll();
			return false;
		}

		hr = m_dsvPool.Initialize(m_device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 256);
		if (FAILED(hr))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "DSV Pool:{}", ResultToInfo(hr));
			CloseAll();
			return false;
		}

		D3D12_INDIRECT_ARGUMENT_DESC argumentDesc = {};
		D3D12_COMMAND_SIGNATURE_DESC signatureDesc = {};
		signatureDesc.NumArgumentDescs = 1;
		signatureDesc.pArgumentDescs = &argumentDesc;

		argumentDesc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;
		signatureDesc.ByteStride = sizeof(D3D12_DRAW_ARGUMENTS);
		hr = m_device->CreateCommandSignature(&signatureDesc, nullptr, IID_PPV_ARGS(&m_drawSignature));
		if (FAILED(hr))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Draw Signature:{}", ResultToInfo(hr));
			CloseAll();
			return false;
		}

		argumentDesc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
		signatureDesc.ByteStride = sizeof(D3D12_DRAW_INDEXED_ARGUMENTS);
		hr = m_device->CreateCommandSignature(&signatureDesc, nullptr, IID_PPV_ARGS(&m_drawIndexedSignature));
		if (FAILED(hr))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "DrawIndexed Signature:{}", ResultToInfo(hr));
			CloseAll();
			return false;
		}

		argumentDesc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;
		signatureDesc.ByteStride = sizeof(D3D12_DISPATCH_ARGUMENTS);
		hr = m_device->CreateCommandSignature(&signatureDesc, nullptr, IID_PPV_ARGS(&m_dispatchSignature));
		if (FAILED(hr))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Dispatch Signature:{}", ResultToInfo(hr));
			CloseAll();
			return false;
		}

		argumentDesc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH_MESH;
		signatureDesc.ByteStride = sizeof(D3D12_DISPATCH_MESH_ARGUMENTS);
		hr = m_device->CreateCommandSignature(&signatureDesc, nullptr, IID_PPV_ARGS(&m_dispatchMeshSignature));
		if (FAILED(hr))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "DispatchMesh Signature:{}", ResultToInfo(hr));
			CloseAll();
			return false;
		}

		for (u32 i = 0; i < static_cast<u32>(QueueType::Count); i++)
		{
			D3D12_COMMAND_QUEUE_DESC queueDesc = {};
			queueDesc.Type = Helpers::ToCommandListType(static_cast<QueueType>(i));
			queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
			queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

			hr = m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_queues[i]));
			if (FAILED(hr))
			{
				Terminal::Error(StringHelper::GetTypeName(this), "CreateCommandQueue {}:{}", i, ResultToInfo(hr));
				CloseAll();
				return false;
			}
		}

		hr = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_idleFence));
		if (FAILED(hr))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Idle Fence:{}", ResultToInfo(hr));
			CloseAll();
			return false;
		}

		// Pop up the errors and warnings on terminal
		RegisterTerminalMessage();

		return true;
	}

	void D3D12Device::CloseAll()
	{
		if (m_idleFence)
		{
			m_idleFence->Release();
			m_idleFence = nullptr;
			m_idleValue = 0;
		}

		for (ID3D12CommandQueue*& pQueue : m_queues)
		{
			if (pQueue)
			{
				pQueue->Release();
				pQueue = nullptr;
			}
		}

		if (m_dispatchMeshSignature)
		{
			m_dispatchMeshSignature->Release();
			m_dispatchMeshSignature = nullptr;
		}

		if (m_dispatchSignature)
		{
			m_dispatchSignature->Release();
			m_dispatchSignature = nullptr;
		}

		if (m_drawIndexedSignature)
		{
			m_drawIndexedSignature->Release();
			m_drawIndexedSignature = nullptr;
		}

		if (m_drawSignature)
		{
			m_drawSignature->Release();
			m_drawSignature = nullptr;
		}

		if (m_allocator)
		{
			m_allocator->Release();
			m_allocator = nullptr;
		}

		if (m_device && m_debug)
		{
			ID3D12DebugDevice* pDebugDevice = nullptr;
			if (SUCCEEDED(m_device->QueryInterface(IID_PPV_ARGS(&pDebugDevice))))
			{
				pDebugDevice->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
				pDebugDevice->Release();
			}
		}

		if (m_infoQueue)
		{
			m_infoQueue->UnregisterMessageCallback(m_callbackCookie);
			m_infoQueue->Release();
			m_infoQueue = nullptr;
			m_callbackCookie = 0;
		}

		if (m_device)
		{
			m_device->Release();
			m_device = nullptr;
		}

		if (m_adapter)
		{
			m_adapter->Release();
			m_adapter = nullptr;
		}

		if (m_factory)
		{
			m_factory->Release();
			m_factory = nullptr;
		}

		if (m_debug)
		{
			m_debug->Release();
			m_debug = nullptr;
		}
	}

	UniquePtr<Buffer> D3D12Device::CreateRHIBuffer(const BufferDesc& desc)
	{
		UniquePtr<D3D12Buffer> pNewBuffer = MakeUnique<D3D12Buffer>();

		pNewBuffer->m_ownerDevice = this;
		pNewBuffer->m_size = desc.size;
		pNewBuffer->m_usage = desc.usage;
		pNewBuffer->m_memory = desc.memory;

		// Correct alignment
		u64 allocSize = desc.size;
		if (HasFlag(desc.usage, BufferUsage::Constant))
			allocSize = (allocSize + D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1) & ~u64(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1);

		D3D12MA::ALLOCATION_DESC allocDesc = {};
		if (desc.memory == MemoryType::GpuOnly)
			allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
		else if (desc.memory == MemoryType::Upload)
			allocDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
		else if (desc.memory == MemoryType::GpuUpload)
			allocDesc.HeapType = D3D12_HEAP_TYPE_GPU_UPLOAD;
		else if (desc.memory == MemoryType::Readback)
			allocDesc.HeapType = D3D12_HEAP_TYPE_READBACK;
		else
			allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

		D3D12_RESOURCE_DESC1 resourceDesc = {};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Alignment = 0;
		resourceDesc.Width = allocSize;
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		b8 canUseUav = desc.memory == MemoryType::GpuOnly || desc.memory == MemoryType::GpuUpload;
		if (HasFlag(desc.usage, BufferUsage::Storage) && canUseUav)
			resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

		D3D12MA::Allocation* pAllocation = nullptr;
		HRESULT hr = m_allocator->CreateResource3(&allocDesc, &resourceDesc, D3D12_BARRIER_LAYOUT_UNDEFINED,
			nullptr, 0, nullptr, &pAllocation, IID_NULL, nullptr);
		if (FAILED(hr))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "CreateResource3:{}", ResultToInfo(hr));
			return nullptr;
		}

		pNewBuffer->m_allocation = pAllocation;
		pNewBuffer->m_resource = pAllocation->GetResource();
		pNewBuffer->m_gpuAddress = pNewBuffer->m_resource->GetGPUVirtualAddress();

		if (desc.memory != MemoryType::GpuOnly)
		{
			D3D12_RANGE emptyRange = {};
			D3D12_RANGE* pReadRange = desc.memory == MemoryType::Readback ? nullptr : &emptyRange;

			void* pMapped = nullptr;
			hr = pNewBuffer->m_resource->Map(0, pReadRange, &pMapped);
			if (FAILED(hr))
			{
				Terminal::Error(StringHelper::GetTypeName(this), "Map: {}", ResultToInfo(hr));
				return nullptr;
			}

			pNewBuffer->m_mappedData = static_cast<u8*>(pMapped);
		}

		return pNewBuffer;
	}

	UniquePtr<Texture> D3D12Device::CreateRHITexture(const TextureDesc& desc)
	{
		if (desc.type == TextureType::TextureCube && desc.arrayLayers % 6 != 0)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Cube texture needs arrayLayers as a multiple of 6, got: {}", desc.arrayLayers);
			return nullptr;
		}

		UniquePtr<D3D12Texture> pNewTexture = MakeUnique<D3D12Texture>();

		pNewTexture->m_ownerDevice = this;
		pNewTexture->m_type = desc.type;
		pNewTexture->m_format = desc.format;
		pNewTexture->m_usage = desc.usage;
		pNewTexture->m_width = desc.width;
		pNewTexture->m_height = desc.type == TextureType::Texture1D ? 1 : desc.height;
		pNewTexture->m_depth = desc.type == TextureType::Texture3D ? desc.depth : 1;
		pNewTexture->m_arrayLayers = desc.type == TextureType::Texture3D ? 1 : desc.arrayLayers;
		pNewTexture->m_mipLevels = desc.mipLevels;
		pNewTexture->m_sampleCount = desc.sampleCount;

		b8 isDepth = Helpers::IsDepthFormat(desc.format);
		b8 isSampled = HasFlag(desc.usage, TextureUsage::Sampled);

		D3D12_RESOURCE_DESC1 resourceDesc = {};
		if (desc.type == TextureType::Texture1D)
			resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
		else if (desc.type == TextureType::Texture3D)
			resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
		else
			resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

		resourceDesc.Alignment = 0;
		resourceDesc.Width = pNewTexture->m_width;
		resourceDesc.Height = pNewTexture->m_height;
		resourceDesc.DepthOrArraySize = static_cast<UINT16>(desc.type == TextureType::Texture3D ? pNewTexture->m_depth : pNewTexture->m_arrayLayers);
		resourceDesc.MipLevels = static_cast<UINT16>(desc.mipLevels);
		resourceDesc.Format = isDepth && isSampled ? Helpers::ToTypelessFormat(desc.format) : Helpers::ToFormat(desc.format);
		resourceDesc.SampleDesc.Count = static_cast<UINT>(desc.sampleCount);
		resourceDesc.SampleDesc.Quality = 0;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		if (HasFlag(desc.usage, TextureUsage::Storage))
			resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

		if (HasFlag(desc.usage, TextureUsage::RenderTarget))
			resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

		if (HasFlag(desc.usage, TextureUsage::DepthStencil))
		{
			resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

			if (!isSampled)
				resourceDesc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
		}

		D3D12_CLEAR_VALUE clearValue = {};
		D3D12_CLEAR_VALUE* pClearValue = nullptr;
		if (HasFlag(desc.usage, TextureUsage::RenderTarget))
		{
			clearValue.Format = Helpers::ToFormat(desc.format);
			clearValue.Color[0] = desc.clearColor[0];
			clearValue.Color[1] = desc.clearColor[1];
			clearValue.Color[2] = desc.clearColor[2];
			clearValue.Color[3] = desc.clearColor[3];
			pClearValue = &clearValue;
		}
		else if (HasFlag(desc.usage, TextureUsage::DepthStencil))
		{
			clearValue.Format = Helpers::ToFormat(desc.format);
			clearValue.DepthStencil.Depth = desc.clearDepth;
			clearValue.DepthStencil.Stencil = desc.clearStencil;
			pClearValue = &clearValue;
		}

		D3D12MA::ALLOCATION_DESC allocDesc = {};
		allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

		D3D12MA::Allocation* pAllocation = nullptr;
		HRESULT hr = m_allocator->CreateResource3(&allocDesc, &resourceDesc, D3D12_BARRIER_LAYOUT_UNDEFINED,
			pClearValue, 0, nullptr, &pAllocation, IID_PPV_ARGS(&pNewTexture->m_resource));
		if (FAILED(hr))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "CreateResource3:{}", ResultToInfo(hr));
			return nullptr;
		}

		pNewTexture->m_allocation = pAllocation;

		return pNewTexture;
	}

	UniquePtr<TextureView> D3D12Device::CreateRHITextureView(const TextureViewDesc& desc)
	{
		if (!desc.pTexture)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "TextureView needs a texture");
			return nullptr;
		}

		D3D12Texture* pTexture = static_cast<D3D12Texture*>(desc.pTexture);

		u32 mipCount = desc.mipCount == kInvalid32 ? pTexture->GetMipLevels() - desc.baseMip : desc.mipCount;
		u32 layerCount = desc.layerCount == kInvalid32 ? pTexture->GetArrayLayers() - desc.baseLayer : desc.layerCount;

		b8 isMipOutOfRange = desc.baseMip >= pTexture->GetMipLevels() || desc.baseMip + mipCount > pTexture->GetMipLevels();
		b8 isLayerOutOfRange = desc.baseLayer >= pTexture->GetArrayLayers() || desc.baseLayer + layerCount > pTexture->GetArrayLayers();
		if (isMipOutOfRange || isLayerOutOfRange)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "View range is out of texture, mips: {}+{}/{}, layers: {}+{}/{}",
				desc.baseMip, mipCount, pTexture->GetMipLevels(), desc.baseLayer, layerCount, pTexture->GetArrayLayers());
			return nullptr;
		}

		UniquePtr<D3D12TextureView> pNewView = MakeUnique<D3D12TextureView>();

		pNewView->m_ownerDevice = this;
		pNewView->m_texture = pTexture;
		pNewView->m_type = desc.type;
		pNewView->m_format = desc.format == TextureFormat::Undefined ? pTexture->GetFormat() : desc.format;
		pNewView->m_aspect = desc.aspect;
		pNewView->m_baseMip = desc.baseMip;
		pNewView->m_mipCount = mipCount;
		pNewView->m_baseLayer = desc.baseLayer;
		pNewView->m_layerCount = layerCount;
		pNewView->m_rtvPool = &m_rtvPool;
		pNewView->m_dsvPool = &m_dsvPool;

		TextureUsage usage = pTexture->GetUsage();

		if (HasFlag(usage, TextureUsage::Sampled))
			pNewView->m_srvDesc = Helpers::ToShaderResourceViewDesc(*pNewView);

		if (HasFlag(usage, TextureUsage::Storage))
			pNewView->m_uavDesc = Helpers::ToUnorderedAccessViewDesc(*pNewView);

		if (HasFlag(usage, TextureUsage::RenderTarget) && mipCount == 1)
		{
			pNewView->m_rtvHandle = m_rtvPool.Allocate();
			if (pNewView->m_rtvHandle.ptr == 0)
				return nullptr;

			D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = Helpers::ToRenderTargetViewDesc(*pNewView);
			m_device->CreateRenderTargetView(pTexture->Handle(), &rtvDesc, pNewView->m_rtvHandle);
		}

		if (HasFlag(usage, TextureUsage::DepthStencil) && mipCount == 1)
		{
			pNewView->m_dsvHandle = m_dsvPool.Allocate();
			if (pNewView->m_dsvHandle.ptr == 0)
				return nullptr;

			D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = Helpers::ToDepthStencilViewDesc(*pNewView);
			m_device->CreateDepthStencilView(pTexture->Handle(), &dsvDesc, pNewView->m_dsvHandle);

			pNewView->m_dsvReadOnlyHandle = m_dsvPool.Allocate();
			if (pNewView->m_dsvReadOnlyHandle.ptr == 0)
				return nullptr;

			b8 hasStencil = pNewView->m_format == TextureFormat::D24_UNORM_S8_UINT || pNewView->m_format == TextureFormat::D32_FLOAT_S8_UINT;
			dsvDesc.Flags = hasStencil ? (D3D12_DSV_FLAG_READ_ONLY_DEPTH | D3D12_DSV_FLAG_READ_ONLY_STENCIL) : D3D12_DSV_FLAG_READ_ONLY_DEPTH;
			m_device->CreateDepthStencilView(pTexture->Handle(), &dsvDesc, pNewView->m_dsvReadOnlyHandle);
		}

		return pNewView;
	}

	UniquePtr<Sampler> D3D12Device::CreateRHISampler(const SamplerDesc& desc)
	{
		UniquePtr<D3D12Sampler> pNewSampler = MakeUnique<D3D12Sampler>();

		pNewSampler->m_ownerDevice = this;
		pNewSampler->m_desc = desc;
		pNewSampler->m_nativeDesc = Helpers::ToSamplerDesc(desc);

		return pNewSampler;
	}

	UniquePtr<ShaderModule> D3D12Device::CreateRHIShaderModule(const ShaderModuleDesc& desc)
	{
		if (desc.byteCode.GetCount() < 4)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "ShaderModule has no bytecode, entry: {}", desc.entryPoint);
			return nullptr;
		}

		const u8* pData = desc.byteCode.GetData();
		if (pData[0] != 'D' || pData[1] != 'X' || pData[2] != 'B' || pData[3] != 'C')
		{
			Terminal::Error(StringHelper::GetTypeName(this), "ShaderModule bytecode is not DXIL, entry: {}", desc.entryPoint);
			return nullptr;
		}

		UniquePtr<D3D12ShaderModule> pNewModule = MakeUnique<D3D12ShaderModule>();

		pNewModule->m_ownerDevice = this;
		pNewModule->m_stage = desc.stage;
		pNewModule->m_entryPoint = desc.entryPoint;
		pNewModule->m_byteCode = List<u8>(desc.byteCode.begin(), desc.byteCode.end());

		return pNewModule;
	}

	UniquePtr<PipelineLayout> D3D12Device::CreateRHIPipelineLayout(const PipelineLayoutDesc& desc)
	{
		if (desc.bindless && !desc.tables.IsEmpty())
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Bindless layout cannot have tables, got: {}", desc.tables.GetCount());
			return nullptr;
		}

		if (desc.pushConstantSize % 4 != 0)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Push constant size must be a multiple of 4, got: {}", desc.pushConstantSize);
			return nullptr;
		}

		if (desc.pushConstantSize / 4 + desc.tables.GetCount() > 64)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Root signature exceeds 64 DWORDs, push: {} tables: {}", desc.pushConstantSize / 4, desc.tables.GetCount());
			return nullptr;
		}

		usize totalRanges = 0;
		for (const DescriptorTableLayout& table : desc.tables)
			totalRanges += table.ranges.GetCount();

		List<D3D12_DESCRIPTOR_RANGE1> ranges;
		ranges.Reserve(totalRanges);

		List<D3D12_ROOT_PARAMETER1> params;
		params.Reserve(desc.tables.GetCount() + 1);

		if (desc.pushConstantSize > 0)
		{
			D3D12_ROOT_PARAMETER1 param = {};
			param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
			param.Constants.ShaderRegister = 0;
			param.Constants.RegisterSpace = D3D12PipelineLayout::kPushConstantSpace;
			param.Constants.Num32BitValues = desc.pushConstantSize / 4;
			param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			params.PushBack(param);
		}

		for (u32 tableIndex = 0; tableIndex < desc.tables.GetCount(); tableIndex++)
		{
			const DescriptorTableLayout& table = desc.tables[tableIndex];
			if (table.ranges.IsEmpty())
			{
				Terminal::Error(StringHelper::GetTypeName(this), "Table {} has no ranges", tableIndex);
				return nullptr;
			}

			b8 isSamplerTable = table.ranges[0].type == DescriptorType::Sampler;
			usize firstRange = ranges.GetCount();

			for (usize rangeIndex = 0; rangeIndex < table.ranges.GetCount(); rangeIndex++)
			{
				const DescriptorRange& range = table.ranges[rangeIndex];

				if ((range.type == DescriptorType::Sampler) != isSamplerTable)
				{
					Terminal::Error(StringHelper::GetTypeName(this), "Table {} mixes samplers with resources", tableIndex);
					return nullptr;
				}

				if (range.count == kInvalid32 && rangeIndex + 1 != table.ranges.GetCount())
				{
					Terminal::Error(StringHelper::GetTypeName(this), "Table {} has an unbounded range that is not the last", tableIndex);
					return nullptr;
				}

				D3D12_DESCRIPTOR_RANGE1 d3dRange = {};
				d3dRange.RangeType = Helpers::ToDescriptorRangeType(range.type);
				d3dRange.NumDescriptors = range.count;
				d3dRange.BaseShaderRegister = range.binding;
				d3dRange.RegisterSpace = tableIndex;
				d3dRange.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE;
				d3dRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
				ranges.PushBack(d3dRange);
			}

			D3D12_ROOT_PARAMETER1 param = {};
			param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			param.DescriptorTable.NumDescriptorRanges = static_cast<UINT>(table.ranges.GetCount());
			param.DescriptorTable.pDescriptorRanges = ranges.GetData() + firstRange;
			param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			params.PushBack(param);
		}

		D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootDesc = {};
		rootDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
		rootDesc.Desc_1_1.NumParameters = static_cast<UINT>(params.GetCount());
		rootDesc.Desc_1_1.pParameters = params.GetData();
		rootDesc.Desc_1_1.NumStaticSamplers = 0;
		rootDesc.Desc_1_1.pStaticSamplers = nullptr;
		rootDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		if (desc.bindless)
			rootDesc.Desc_1_1.Flags |= D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED | D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED;

		ID3DBlob* pBlob = nullptr;
		ID3DBlob* pError = nullptr;
		HRESULT hr = D3D12SerializeVersionedRootSignature(&rootDesc, &pBlob, &pError);
		if (FAILED(hr))
		{
			std::string reason = pError ? std::string(static_cast<const char*>(pError->GetBufferPointer()), pError->GetBufferSize()) : ResultToInfo(hr);
			Terminal::Error(StringHelper::GetTypeName(this), "D3D12SerializeVersionedRootSignature:{}", reason);

			if (pError)
				pError->Release();

			return nullptr;
		}

		if (pError)
			pError->Release();

		UniquePtr<D3D12PipelineLayout> pNewLayout = MakeUnique<D3D12PipelineLayout>();

		pNewLayout->m_ownerDevice = this;
		pNewLayout->m_tables = desc.tables;
		pNewLayout->m_pushConstantSize = desc.pushConstantSize;
		pNewLayout->m_bindless = desc.bindless;
		pNewLayout->m_tableRootOffset = desc.pushConstantSize > 0 ? 1 : 0;

		hr = m_device->CreateRootSignature(0, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), IID_PPV_ARGS(&pNewLayout->m_rootSignature));
		pBlob->Release();
		if (FAILED(hr))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "CreateRootSignature:{}", ResultToInfo(hr));
			return nullptr;
		}

		return pNewLayout;
	}

	UniquePtr<GraphicsPipeline> D3D12Device::CreateRHIPipeline(const GraphicsPipelineDesc& desc)
	{
		if (!desc.pLayout)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "GraphicsPipeline needs a layout");
			return nullptr;
		}

		b8 isMeshPipeline = desc.pMeshShader != nullptr;
		if (isMeshPipeline == (desc.pVertexShader != nullptr))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "GraphicsPipeline needs either a vertex or a mesh shader, not both or none");
			return nullptr;
		}

		if (desc.colorTargets.GetCount() > D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "GraphicsPipeline has {} color targets, max: {}", desc.colorTargets.GetCount(), D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT);
			return nullptr;
		}

		List<D3D12_INPUT_ELEMENT_DESC> inputElements;
		inputElements.Reserve(desc.vertexLayout.attributes.GetCount());

		for (const VertexAttribute& attribute : desc.vertexLayout.attributes)
		{
			VertexInputRate inputRate = VertexInputRate::PerVertex;
			for (const VertexBinding& binding : desc.vertexLayout.bindings)
			{
				if (binding.binding == attribute.binding)
					inputRate = binding.inputRate;
			}

			b8 isPerInstance = inputRate == VertexInputRate::PerInstance;

			D3D12_INPUT_ELEMENT_DESC element = {};
			element.SemanticName = attribute.semantic.c_str();
			element.SemanticIndex = attribute.semanticIndex;
			element.Format = Helpers::ToFormat(attribute.format);
			element.InputSlot = attribute.binding;
			element.AlignedByteOffset = attribute.offset;
			element.InputSlotClass = isPerInstance ? D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA : D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			element.InstanceDataStepRate = isPerInstance ? 1 : 0;
			inputElements.PushBack(element);
		}

		D3D12GraphicsPipelineStream stream = {};
		stream.rootSignature.desc = static_cast<D3D12PipelineLayout*>(desc.pLayout)->Handle();

		if (desc.pVertexShader)
			stream.vertexShader.desc = static_cast<D3D12ShaderModule*>(desc.pVertexShader)->Bytecode();

		if (desc.pPixelShader)
			stream.pixelShader.desc = static_cast<D3D12ShaderModule*>(desc.pPixelShader)->Bytecode();

		if (desc.pTaskShader)
			stream.taskShader.desc = static_cast<D3D12ShaderModule*>(desc.pTaskShader)->Bytecode();

		if (desc.pMeshShader)
			stream.meshShader.desc = static_cast<D3D12ShaderModule*>(desc.pMeshShader)->Bytecode();

		stream.inputLayout.desc.pInputElementDescs = inputElements.GetData();
		stream.inputLayout.desc.NumElements = static_cast<UINT>(inputElements.GetCount());
		stream.topology.desc = Helpers::ToPrimitiveTopologyType(desc.topology);

		stream.rasterizer.desc = Helpers::ToRasterizerDesc(desc.rasterizer);
		stream.depthStencil.desc = Helpers::ToDepthStencilDesc(desc.depthStencil);
		stream.blend.desc = Helpers::ToBlendDesc(desc.colorTargets, desc.alphaToCoverage);

		stream.renderTargetFormats.desc.NumRenderTargets = static_cast<UINT>(desc.colorTargets.GetCount());
		for (usize i = 0; i < desc.colorTargets.GetCount(); i++)
			stream.renderTargetFormats.desc.RTFormats[i] = Helpers::ToFormat(desc.colorTargets[i].format);

		stream.depthStencilFormat.desc = Helpers::ToFormat(desc.depthFormat);
		stream.sampleDesc.desc.Count = static_cast<UINT>(desc.sampleCount);
		stream.sampleDesc.desc.Quality = 0;
		stream.sampleMask.desc = UINT_MAX;

		D3D12_PIPELINE_STATE_STREAM_DESC streamDesc = {};
		streamDesc.SizeInBytes = sizeof(stream);
		streamDesc.pPipelineStateSubobjectStream = &stream;

		UniquePtr<D3D12GraphicsPipeline> pNewPipeline = MakeUnique<D3D12GraphicsPipeline>();

		pNewPipeline->m_ownerDevice = this;
		pNewPipeline->m_type = PipelineType::Graphics;
		pNewPipeline->m_layout = desc.pLayout;
		pNewPipeline->m_topology = desc.topology;
		pNewPipeline->m_vertexBindings = desc.vertexLayout.bindings;
		pNewPipeline->m_isMeshPipeline = isMeshPipeline;

		HRESULT hr = m_device->CreatePipelineState(&streamDesc, IID_PPV_ARGS(&pNewPipeline->m_pipelineState));
		if (FAILED(hr))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "CreatePipelineState:{}", ResultToInfo(hr));
			return nullptr;
		}

		return pNewPipeline;
	}

	UniquePtr<ComputePipeline> D3D12Device::CreateRHIPipeline(const ComputePipelineDesc& desc)
	{
		if (!desc.pLayout || !desc.pComputeShader)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "ComputePipeline needs a layout and a compute shader");
			return nullptr;
		}

		D3D12_COMPUTE_PIPELINE_STATE_DESC pipelineDesc = {};
		pipelineDesc.pRootSignature = static_cast<D3D12PipelineLayout*>(desc.pLayout)->Handle();
		pipelineDesc.CS = static_cast<D3D12ShaderModule*>(desc.pComputeShader)->Bytecode();

		UniquePtr<D3D12ComputePipeline> pNewPipeline = MakeUnique<D3D12ComputePipeline>();

		pNewPipeline->m_ownerDevice = this;
		pNewPipeline->m_type = PipelineType::Compute;
		pNewPipeline->m_layout = desc.pLayout;

		HRESULT hr = m_device->CreateComputePipelineState(&pipelineDesc, IID_PPV_ARGS(&pNewPipeline->m_pipelineState));
		if (FAILED(hr))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "CreateComputePipelineState:{}", ResultToInfo(hr));
			return nullptr;
		}

		return pNewPipeline;
	}

	UniquePtr<DescriptorHeap> D3D12Device::CreateRHIDescriptorHeap(const DescriptorHeapDesc& desc)
	{
		if (desc.capacity == 0)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Descriptor heap capacity is zero");
			return nullptr;
		}

		if (desc.type == DescriptorHeapType::Sampler && desc.capacity > D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Sampler heap capacity {} exceeds {}", desc.capacity, D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE);
			return nullptr;
		}

		UniquePtr<D3D12DescriptorHeap> pNewHeap = MakeUnique<D3D12DescriptorHeap>();

		pNewHeap->m_ownerDevice = this;
		pNewHeap->m_type = desc.type;
		pNewHeap->m_capacity = desc.capacity;
		pNewHeap->m_device = m_device;

		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.Type = desc.type == DescriptorHeapType::Sampler ? D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER : D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		heapDesc.NumDescriptors = desc.capacity;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

		HRESULT hr = m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&pNewHeap->m_heap));
		if (FAILED(hr))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "CreateDescriptorHeap:{}", ResultToInfo(hr));
			return nullptr;
		}

		pNewHeap->m_cpuStart = pNewHeap->m_heap->GetCPUDescriptorHandleForHeapStart();
		pNewHeap->m_gpuStart = pNewHeap->m_heap->GetGPUDescriptorHandleForHeapStart();
		pNewHeap->m_increment = m_device->GetDescriptorHandleIncrementSize(heapDesc.Type);
		pNewHeap->m_freeBlocks.PushBack({ 0, desc.capacity });

		return pNewHeap;
	}

	UniquePtr<DescriptorTable> D3D12Device::CreateRHIDescriptorTable(const DescriptorTableDesc& desc)
	{
		if (!desc.pHeap || !desc.pLayout)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "DescriptorTable needs a heap and a layout");
			return nullptr;
		}

		const List<DescriptorTableLayout>& tables = desc.pLayout->GetTables();
		if (desc.tableIndex >= tables.GetCount())
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Layout has no table {}, table count: {}", desc.tableIndex, tables.GetCount());
			return nullptr;
		}

		const DescriptorTableLayout& layout = tables[desc.tableIndex];
		b8 isSamplerTable = layout.ranges[0].type == DescriptorType::Sampler;
		b8 isSamplerHeap = desc.pHeap->GetType() == DescriptorHeapType::Sampler;
		if (isSamplerTable != isSamplerHeap)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Table {} does not match the heap type", desc.tableIndex);
			return nullptr;
		}

		UniquePtr<D3D12DescriptorTable> pNewTable = MakeUnique<D3D12DescriptorTable>();

		pNewTable->m_ownerDevice = this;
		pNewTable->m_heap = desc.pHeap;
		pNewTable->m_layout = desc.pLayout;
		pNewTable->m_tableIndex = desc.tableIndex;
		pNewTable->m_variableCount = desc.variableCount;

		u32 descriptorCount = 0;
		for (const DescriptorRange& range : layout.ranges)
		{
			u32 count = range.count == kInvalid32 ? desc.variableCount : range.count;
			pNewTable->m_slots.PushBack({ range.binding, range.type, descriptorCount, count });
			descriptorCount += count;
		}

		if (descriptorCount == 0)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Table {} has no descriptors, variableCount: {}", desc.tableIndex, desc.variableCount);
			return nullptr;
		}

		u32 heapOffset = static_cast<D3D12DescriptorHeap*>(desc.pHeap)->AllocateRange(descriptorCount);
		if (heapOffset == kInvalid32)
			return nullptr;

		pNewTable->m_offset = heapOffset;
		pNewTable->m_count = descriptorCount;

		return pNewTable;
	}

	UniquePtr<CommandPool> D3D12Device::CreateRHICommandPool(const CommandPoolDesc& desc)
	{
		UniquePtr<D3D12CommandPool> pNewPool = MakeUnique<D3D12CommandPool>();

		pNewPool->m_ownerDevice = this;
		pNewPool->m_type = desc.type;

		HRESULT hr = m_device->CreateCommandAllocator(Helpers::ToCommandListType(desc.type), IID_PPV_ARGS(&pNewPool->m_allocator));
		if (FAILED(hr))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "CreateCommandAllocator:{}", ResultToInfo(hr));
			return nullptr;
		}

		return pNewPool;
	}

	UniquePtr<CommandList> D3D12Device::CreateRHICommandList(const CommandListDesc& desc)
	{
		if (!desc.pPool)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "CommandList needs a pool");
			return nullptr;
		}

		UniquePtr<D3D12CommandList> pNewList = MakeUnique<D3D12CommandList>();

		pNewList->m_ownerDevice = this;
		pNewList->m_pool = desc.pPool;
		pNewList->m_type = desc.pPool->GetType();
		pNewList->m_d3dDevice = this;

		HRESULT hr = m_device->CreateCommandList1(0, Helpers::ToCommandListType(pNewList->m_type), D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&pNewList->m_list));
		if (FAILED(hr))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "CreateCommandList1:{}", ResultToInfo(hr));
			return nullptr;
		}

		return pNewList;
	}

	UniquePtr<Fence> D3D12Device::CreateRHIFence(const FenceDesc& desc)
	{
		UniquePtr<D3D12Fence> pNewFence = MakeUnique<D3D12Fence>();

		pNewFence->m_ownerDevice = this;
		pNewFence->m_value = desc.initialValue;

		HRESULT hr = m_device->CreateFence(desc.initialValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pNewFence->m_fence));
		if (FAILED(hr))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "CreateFence:{}", ResultToInfo(hr));
			return nullptr;
		}

		return pNewFence;
	}

	UniquePtr<Swapchain> D3D12Device::CreateRHISwapchain(const SwapchainDesc& desc)
	{
		if (desc.windowHandle == PAL::OSHandle{} || desc.width == 0 || desc.height == 0)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Swapchain needs a window and a size, size: {}x{}", desc.width, desc.height);
			return nullptr;
		}

		if (desc.imageCount < 2 || desc.imageCount > kMaxSwapchainImages)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Swapchain image count must be in [2, {}], got: {}", kMaxSwapchainImages, desc.imageCount);
			return nullptr;
		}

		UniquePtr<D3D12Swapchain> pNewSwapchain = MakeUnique<D3D12Swapchain>();

		pNewSwapchain->m_ownerDevice = this;
		pNewSwapchain->m_format = desc.format;
		pNewSwapchain->m_presentMode = desc.presentMode;
		pNewSwapchain->m_width = desc.width;
		pNewSwapchain->m_height = desc.height;
		pNewSwapchain->m_imageCount = desc.imageCount;

		BOOL tearingSupported = FALSE;
		m_factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &tearingSupported, sizeof(tearingSupported));
		pNewSwapchain->m_tearingSupported = tearingSupported == TRUE;

		DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
		swapchainDesc.Width = desc.width;
		swapchainDesc.Height = desc.height;
		swapchainDesc.Format = Helpers::ToFormat(desc.format);
		swapchainDesc.Stereo = FALSE;
		swapchainDesc.SampleDesc.Count = 1;
		swapchainDesc.SampleDesc.Quality = 0;
		swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapchainDesc.BufferCount = desc.imageCount;
		swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
		swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;

		if (pNewSwapchain->m_tearingSupported)
			swapchainDesc.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

		HWND hwnd = reinterpret_cast<HWND>(static_cast<uptr>(desc.windowHandle));

		IDXGISwapChain1* pBaseSwapchain = nullptr;
		HRESULT hr = m_factory->CreateSwapChainForHwnd(m_queues[static_cast<usize>(QueueType::Graphics)], hwnd, &swapchainDesc, nullptr, nullptr, &pBaseSwapchain);
		if (FAILED(hr))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "CreateSwapChainForHwnd:{}", ResultToInfo(hr));
			return nullptr;
		}

		hr = pBaseSwapchain->QueryInterface(IID_PPV_ARGS(&pNewSwapchain->m_swapchain));
		pBaseSwapchain->Release();
		if (FAILED(hr))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "QueryInterface To IDXGISwapChain4:{}", ResultToInfo(hr));
			return nullptr;
		}

		m_factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);

		pNewSwapchain->m_swapchainFlags = swapchainDesc.Flags;
		pNewSwapchain->m_swapchain->SetMaximumFrameLatency(desc.imageCount - 1);
		pNewSwapchain->m_waitableObject = pNewSwapchain->m_swapchain->GetFrameLatencyWaitableObject();

		if (!pNewSwapchain->CreateImages())
			return nullptr;

		pNewSwapchain->m_imageIndex = pNewSwapchain->m_swapchain->GetCurrentBackBufferIndex();

		return pNewSwapchain;
	}

	void D3D12Device::Submit(QueueType type, ReadOnlyList<CommandList*> lists)
	{
		constexpr usize kMaxSubmitLists = 64;
		if (lists.GetCount() > kMaxSubmitLists)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Submit batch limit exceeded, {} > {}", lists.GetCount(), kMaxSubmitLists);
			return;
		}

		ID3D12CommandList* nativeLists[kMaxSubmitLists] = {};
		for (usize i = 0; i < lists.GetCount(); i++)
		{
			if (lists[i]->GetType() != type)
			{
				Terminal::Error(StringHelper::GetTypeName(this), "Command list {} does not match the queue type", i);
				return;
			}

			nativeLists[i] = static_cast<D3D12CommandList*>(lists[i])->Handle();
		}

		m_queues[static_cast<usize>(type)]->ExecuteCommandLists(static_cast<UINT>(lists.GetCount()), nativeLists);
	}

	u64 D3D12Device::Signal(QueueType type, Fence* pFence)
	{
		D3D12Fence* pD3DFence = static_cast<D3D12Fence*>(pFence);
		u64 value = pD3DFence->Advance();

		HRESULT hr = m_queues[static_cast<usize>(type)]->Signal(pD3DFence->Handle(), value);
		if (FAILED(hr))
			Terminal::Error(StringHelper::GetTypeName(this), "Queue Signal:{}", ResultToInfo(hr));

		return value;
	}

	void D3D12Device::Wait(QueueType type, Fence* pFence, u64 value)
	{
		HRESULT hr = m_queues[static_cast<usize>(type)]->Wait(static_cast<D3D12Fence*>(pFence)->Handle(), value);
		if (FAILED(hr))
			Terminal::Error(StringHelper::GetTypeName(this), "Queue Wait:{}", ResultToInfo(hr));
	}

	void D3D12Device::WaitIdle()
	{
		for (ID3D12CommandQueue* pQueue : m_queues)
		{
			u64 value = ++m_idleValue;

			HRESULT hr = pQueue->Signal(m_idleFence, value);
			if (FAILED(hr))
			{
				Terminal::Error(StringHelper::GetTypeName(this), "WaitIdle Signal:{}", ResultToInfo(hr));
				continue;
			}

			hr = m_idleFence->SetEventOnCompletion(value, nullptr);
			if (FAILED(hr))
				Terminal::Error(StringHelper::GetTypeName(this), "WaitIdle SetEventOnCompletion:{}", ResultToInfo(hr));
		}
	}

	u64 D3D12Device::GetTimestampFrequency(QueueType type)
	{
		UINT64 frequency = 0;

		HRESULT hr = m_queues[static_cast<usize>(type)]->GetTimestampFrequency(&frequency);
		if (FAILED(hr))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "GetTimestampFrequency:{}", ResultToInfo(hr));
			return 0;
		}

		return frequency;
	}

	void D3D12Device::InitializeImGui(u32 framesInFlight, DescriptorHeap* pResourceHeap, TextureFormat colorFormat)
	{
		if (!pResourceHeap || pResourceHeap->GetType() != DescriptorHeapType::Resource)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "ImGui needs a resource descriptor heap");
			return;
		}

		D3D12DescriptorHeap* pD3DHeap = static_cast<D3D12DescriptorHeap*>(pResourceHeap);

		ImGui_ImplDX12_InitInfo info = {};
		info.Device = m_device;
		info.CommandQueue = m_queues[static_cast<usize>(QueueType::Graphics)];
		info.NumFramesInFlight = static_cast<i32>(framesInFlight);
		info.RTVFormat = Helpers::ToFormat(colorFormat);
		info.DSVFormat = DXGI_FORMAT_UNKNOWN;
		info.SrvDescriptorHeap = pD3DHeap->Handle();
		info.SrvDescriptorAllocFn = &ImGuiAlloc;
		info.SrvDescriptorFreeFn = &ImGuiFree;
		info.UserData = pD3DHeap;

		if (!ImGui_ImplDX12_Init(&info))
			Terminal::Error(StringHelper::GetTypeName(this), "ImGui_ImplDX12_Init failed");
	}

	void D3D12Device::NewFrameImGui()
	{
		ImGui_ImplDX12_NewFrame();
	}

	void D3D12Device::ShutdownImGui()
	{
		ImGui_ImplDX12_Shutdown();
	}

	void D3D12Device::RegisterTerminalMessage()
	{
		if (FAILED(m_device->QueryInterface(IID_PPV_ARGS(&m_infoQueue))))
			return;

		auto cb = [](D3D12_MESSAGE_CATEGORY, D3D12_MESSAGE_SEVERITY sev,
			D3D12_MESSAGE_ID, LPCSTR pDesc, void*)
			{
				switch (sev)
				{
				case D3D12_MESSAGE_SEVERITY_CORRUPTION:
					Terminal::Error("D3D12", "CORRUPTION: {}", pDesc);
					break;
				case D3D12_MESSAGE_SEVERITY_ERROR:
					Terminal::Error("D3D12", "ERROR: {}", pDesc);
					break;
				case D3D12_MESSAGE_SEVERITY_WARNING:
					Terminal::Warn("D3D12", "WARNING: {}", pDesc);
					break;
				case D3D12_MESSAGE_SEVERITY_INFO:
					Terminal::Debug("D3D12", "INFO: {}", pDesc);
					break;
				default:
					Terminal::Debug("D3D12", "MESSAGE: {}", pDesc);
					break;
				}
			};

		m_infoQueue->RegisterMessageCallback(cb, D3D12_MESSAGE_CALLBACK_FLAG_NONE, nullptr, &m_callbackCookie);
	}

}