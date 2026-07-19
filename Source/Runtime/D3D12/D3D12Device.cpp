#include "D3D12Device.h"

#include <Runtime/D3D12/D3D12Buffer.h>
#include <Runtime/D3D12/D3D12CommandList.h>
#include <Runtime/D3D12/D3D12DescriptorHeap.h>
#include <Runtime/D3D12/D3D12Fence.h>
#include <Runtime/D3D12/D3D12Queue.h>
#include <Runtime/D3D12/D3D12Sampler.h>
#include <Runtime/D3D12/D3D12Swapchain.h>
#include <Runtime/D3D12/D3D12Texture.h>

#include <imgui.h>
#include <backends/imgui_impl_dx12.h>

namespace Horizon
{
	namespace
	{
		void ImGuiAlloc(ImGui_ImplDX12_InitInfo* pInfo, D3D12_CPU_DESCRIPTOR_HANDLE* pOutCpu, D3D12_GPU_DESCRIPTOR_HANDLE* pOutGpu)
		{
			auto* pDevice = static_cast<D3D12Device*>(pInfo->UserData);
			auto* pHeap = static_cast<D3D12DescriptorHeap*>(pDevice->GetDescriptorHeap(GfxDescriptorHeapType::Resource));

			const u32 index = pHeap->Allocate();

			*pOutCpu = pHeap->CpuAt(index);
			*pOutGpu = pHeap->GpuAt(index);
		}

		void ImGuiFree(ImGui_ImplDX12_InitInfo* pInfo, D3D12_CPU_DESCRIPTOR_HANDLE cpu, D3D12_GPU_DESCRIPTOR_HANDLE)
		{
			auto* pDevice = static_cast<D3D12Device*>(pInfo->UserData);
			auto* pHeap = static_cast<D3D12DescriptorHeap*>(pDevice->GetDescriptorHeap(GfxDescriptorHeapType::Resource));

			pHeap->Free(pHeap->IndexOf(cpu));
		}
	}

	GfxDevice* CreateGfxDevice(const GfxDeviceDesc& desc)
	{
		auto* pDevice = Allocator::Create<D3D12Device>(CurrLoc());

		pDevice->Init(desc);
		return pDevice;
	}

	void D3D12Device::Init(const GfxDeviceDesc& desc)
	{
		m_desc = desc;

		u32 factoryFlags = 0;
#if defined(HORIZON_DEBUG)
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&m_debug))))
		{
			m_debug->EnableDebugLayer();
			factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
#endif

		HRESULT hr = CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&m_factory));
		CHECK_HR(hr, "IDXGIFactory7 - CreateDXGIFactory2");

		hr = m_factory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
			IID_PPV_ARGS(&m_adapter));
		CHECK_HR(hr, "IDXGIAdapter4 - EnumAdapterByGpuPreference");

		ID3D12Device* pBase = nullptr;

		hr = D3D12CreateDevice(m_adapter, D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&pBase));
		CHECK_HR(hr, "ID3D12Device - D3D12CreateDevice");

		hr = pBase->QueryInterface(IID_PPV_ARGS(&m_device));
		CHECK_HR(hr, "ID3D12Device10 - QueryInterface");
		pBase->Release();

		D3D12MA::ALLOCATOR_DESC allocDesc = {};

		allocDesc.pDevice = m_device;
		allocDesc.pAdapter = m_adapter;
		D3D12MA::CreateAllocator(&allocDesc, &m_allocator);

#if defined(HORIZON_DEBUG)
		CreateTerminalLog();
#endif

		const GfxDescriptorBudget& budget = m_desc.descriptorBudget;
		u32 samplerCapacity = budget.samplerCount;

		if (samplerCapacity > 2048)
		{
			Terminal::Warn("D3D12Device", "Sampler budget {} exceeds D3D12 limit, clamping to 2048", samplerCapacity);
			samplerCapacity = 2048;
		}

		m_resourceHeap = CreateDescriptorHeap(GfxDescriptorHeapType::Resource, budget.resourceCount, true);
		m_samplerHeap = CreateDescriptorHeap(GfxDescriptorHeapType::Sampler, samplerCapacity, true);
		m_colorHeap = CreateDescriptorHeap(GfxDescriptorHeapType::Color, budget.colorCount, false);
		m_depthHeap = CreateDescriptorHeap(GfxDescriptorHeapType::Depth, budget.depthCount, false);

		CreateRootSignature();
		CreateCommandSignatures();
	}

	D3D12Device::~D3D12Device()
	{
		FlushPendingDeletes(kInvalid64);

		Allocator::Delete(m_resourceHeap);
		Allocator::Delete(m_samplerHeap);
		Allocator::Delete(m_colorHeap);
		Allocator::Delete(m_depthHeap);

		if (m_drawSignature)
			m_drawSignature->Release();

		if (m_drawIndexedSignature)
			m_drawIndexedSignature->Release();

		if (m_dispatchSignature)
			m_dispatchSignature->Release();

		if (m_rootSignature)
			m_rootSignature->Release();

		if (m_allocator)
			m_allocator->Release();

#if defined(HORIZON_DEBUG)
		if (m_infoQueue)
		{
			m_infoQueue->UnregisterMessageCallback(m_callbackCookie);
			m_infoQueue->Release();
		}
#endif

		if (m_device)
			m_device->Release();

		if (m_debug)
			m_debug->Release();

		if (m_adapter)
			m_adapter->Release();

		if (m_factory)
			m_factory->Release();
	}

	void D3D12Device::InitializeImGui(GfxQueue* pQueue, GfxTextureFormat fmt)
	{
		auto* pD3DQueue = static_cast<D3D12Queue*>(pQueue);

		ImGui_ImplDX12_InitInfo info = {};

		info.Device = m_device;
		info.CommandQueue = pD3DQueue->Handle();
		info.NumFramesInFlight = i32(m_desc.framesInFlight);
		info.RTVFormat = Helpers::ToDXGIFormat(fmt);
		info.DSVFormat = DXGI_FORMAT_UNKNOWN;
		info.SrvDescriptorHeap = m_resourceHeap->Handle();
		info.SrvDescriptorAllocFn = &ImGuiAlloc;
		info.SrvDescriptorFreeFn = &ImGuiFree;
		info.UserData = this;

		ImGui_ImplDX12_Init(&info);
	}

	void D3D12Device::ShutdownImGui()
	{
		ImGui_ImplDX12_Shutdown();
	}

	GfxTexture* D3D12Device::CreateTexture(const GfxTextureDesc& desc)
	{
		const DXGI_FORMAT viewFormat = Helpers::ToDXGIFormat(desc.format);
		const b8 isDepth = Helpers::IsDepthFormat(desc.format);
		const b8 isSampled = HasFlag(desc.usage, GfxTextureUsage::Sampled);

		D3D12_RESOURCE_DESC resourceDesc = {};

		resourceDesc.Dimension = Helpers::ToResourceDimension(desc.type);
		resourceDesc.Width = desc.width;
		resourceDesc.Height = desc.height;
		resourceDesc.DepthOrArraySize = desc.type == GfxTextureType::Tex3D ? u16(desc.depth) : u16(desc.arraySize);
		resourceDesc.MipLevels = u16(desc.mipLevels);
		resourceDesc.Format = (isDepth && isSampled) ? Helpers::ToTypelessFormat(viewFormat) : viewFormat;
		resourceDesc.SampleDesc = { Helpers::ToSampleCount(desc.sampleCount), 0 };
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resourceDesc.Flags = Helpers::ToResourceFlags(desc.usage);

		D3D12_CLEAR_VALUE clear = {};

		clear.Format = viewFormat;

		if (isDepth)
		{
			clear.DepthStencil = { desc.clearDepth, desc.clearStencil };
		}
		else
		{
			clear.Color[0] = desc.clearColor.r;
			clear.Color[1] = desc.clearColor.g;
			clear.Color[2] = desc.clearColor.b;
			clear.Color[3] = desc.clearColor.a;
		}

		const b8 wantsClear = HasFlag(desc.usage, GfxTextureUsage::RenderTarget)
			|| HasFlag(desc.usage, GfxTextureUsage::DepthStencil);

		D3D12MA::ALLOCATION_DESC allocDesc = {};

		allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

		auto* pTex = Allocator::Create<D3D12Texture>(CurrLoc());

		pTex->m_ownerDevice = this;

		HRESULT hr = m_allocator->CreateResource(&allocDesc, &resourceDesc, D3D12_RESOURCE_STATE_COMMON,
			wantsClear ? &clear : nullptr, &pTex->m_allocation, IID_PPV_ARGS(&pTex->m_resource));
		CHECK_REASON(hr, "ID3D12Resource - CreateResource (Texture)");

		if (FAILED(hr))
		{
			Allocator::Delete(pTex);
			return nullptr;
		}

		pTex->m_desc = desc;
		pTex->m_state = GfxResourceState::Common;
		pTex->m_dxgiFormat = viewFormat;

		if (isSampled)
			CreateTextureSRV(pTex);

		if (HasFlag(desc.usage, GfxTextureUsage::Storage))
			CreateTextureUAV(pTex);

		if (HasFlag(desc.usage, GfxTextureUsage::RenderTarget))
			CreateTextureRTV(pTex);

		if (HasFlag(desc.usage, GfxTextureUsage::DepthStencil))
			CreateTextureDSV(pTex);

		return pTex;
	}

	GfxBuffer* D3D12Device::CreateBuffer(const GfxBufferDesc& desc)
	{
		usize size = desc.size;

		if (HasFlag(desc.usage, GfxBufferUsage::Constant))
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
		resourceDesc.Flags = Helpers::ToResourceFlags(desc.usage);

		D3D12MA::ALLOCATION_DESC allocDesc = {};

		allocDesc.HeapType = Helpers::ToHeapType(desc.memory);

		const D3D12_RESOURCE_STATES initialState =
			desc.memory == GfxMemoryType::Upload ? D3D12_RESOURCE_STATE_GENERIC_READ :
			desc.memory == GfxMemoryType::Readback ? D3D12_RESOURCE_STATE_COPY_DEST :
			D3D12_RESOURCE_STATE_COMMON;

		auto* pBuf = Allocator::Create<D3D12Buffer>(CurrLoc());

		pBuf->m_ownerDevice = this;

		HRESULT hr = m_allocator->CreateResource(&allocDesc, &resourceDesc, initialState, nullptr,
			&pBuf->m_allocation, IID_PPV_ARGS(&pBuf->m_resource));
		CHECK_REASON(hr, "ID3D12Resource - CreateResource (Buffer)");

		if (FAILED(hr))
		{
			Allocator::Delete(pBuf);
			return nullptr;
		}

		pBuf->m_desc = desc;
		pBuf->m_gpuAddress = pBuf->m_resource->GetGPUVirtualAddress();

		if (desc.memory != GfxMemoryType::GpuOnly)
		{
			D3D12_RANGE noRead = { 0, 0 };

			pBuf->m_resource->Map(0, &noRead, &pBuf->m_mapped);
		}

		const b8 shaderRead = HasFlag(desc.usage, GfxBufferUsage::Vertex)
			|| HasFlag(desc.usage, GfxBufferUsage::Index)
			|| HasFlag(desc.usage, GfxBufferUsage::Storage);

		if (shaderRead && desc.memory == GfxMemoryType::GpuOnly)
			CreateBufferSRV(pBuf);

		if (HasFlag(desc.usage, GfxBufferUsage::Storage))
			CreateBufferUAV(pBuf);

		return pBuf;
	}

	GfxSampler* D3D12Device::CreateSampler(const GfxSamplerDesc& desc)
	{
		const u32 index = m_samplerHeap->Allocate();

		if (index == kInvalid32)
		{
			Terminal::Error("D3D12Device", "Sampler heap is exhausted");
			return nullptr;
		}

		D3D12_SAMPLER_DESC samplerDesc = {};

		samplerDesc.Filter = Helpers::ToFilter(desc.minFilter, desc.magFilter, desc.mipFilter,
			desc.anisotropyEnable, desc.compareEnable);
		samplerDesc.AddressU = Helpers::ToAddressMode(desc.addressU);
		samplerDesc.AddressV = Helpers::ToAddressMode(desc.addressV);
		samplerDesc.AddressW = Helpers::ToAddressMode(desc.addressW);
		samplerDesc.MipLODBias = desc.mipLodBias;
		samplerDesc.MaxAnisotropy = desc.anisotropyEnable ? desc.maxAnisotropy : 1;
		samplerDesc.ComparisonFunc = Helpers::ToCompare(desc.compareOp);
		samplerDesc.MinLOD = desc.minLod;
		samplerDesc.MaxLOD = desc.maxLod;

		switch (desc.borderColor)
		{
		case GfxBorderColor::TransparentBlack:
			samplerDesc.BorderColor[0] = 0.0f;
			samplerDesc.BorderColor[1] = 0.0f;
			samplerDesc.BorderColor[2] = 0.0f;
			samplerDesc.BorderColor[3] = 0.0f;
			break;
		case GfxBorderColor::OpaqueBlack:
			samplerDesc.BorderColor[0] = 0.0f;
			samplerDesc.BorderColor[1] = 0.0f;
			samplerDesc.BorderColor[2] = 0.0f;
			samplerDesc.BorderColor[3] = 1.0f;
			break;
		case GfxBorderColor::OpaqueWhite:
			samplerDesc.BorderColor[0] = 1.0f;
			samplerDesc.BorderColor[1] = 1.0f;
			samplerDesc.BorderColor[2] = 1.0f;
			samplerDesc.BorderColor[3] = 1.0f;
			break;
		}

		m_device->CreateSampler(&samplerDesc, m_samplerHeap->CpuAt(index));

		auto* pSampler = Allocator::Create<D3D12Sampler>(CurrLoc());

		pSampler->m_ownerDevice = this;
		pSampler->m_desc = desc;
		pSampler->m_heapIndex = index;

		return pSampler;
	}

	GfxQueue* D3D12Device::CreateQueue(GfxQueueType type)
	{
		auto* pQueue = Allocator::Create<D3D12Queue>(CurrLoc());

		pQueue->m_ownerDevice = this;
		pQueue->m_type = type;

		D3D12_COMMAND_QUEUE_DESC queueDesc = {};

		queueDesc.Type = Helpers::ToListType(type);

		HRESULT hr = m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&pQueue->m_queue));
		CHECK_REASON(hr, "ID3D12CommandQueue - CreateCommandQueue");

		return pQueue;
	}

	GfxFence* D3D12Device::CreateFence()
	{
		auto* pFence = Allocator::Create<D3D12Fence>(CurrLoc());

		pFence->m_ownerDevice = this;

		HRESULT hr = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pFence->m_fence));
		CHECK_REASON(hr, "ID3D12Fence - CreateFence");

		return pFence;
	}

	GfxCommandList* D3D12Device::CreateCommandList(GfxQueueType type)
	{
		auto* pCmd = Allocator::Create<D3D12CommandList>(CurrLoc());

		pCmd->m_ownerDevice = this;
		pCmd->m_device = this;
		pCmd->m_queueType = type;
		pCmd->m_type = Helpers::ToListType(type);

		m_device->CreateCommandAllocator(pCmd->m_type, IID_PPV_ARGS(&pCmd->m_allocator));
		m_device->CreateCommandList1(0, pCmd->m_type, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&pCmd->m_list));

		return pCmd;
	}

	GfxSwapchain* D3D12Device::CreateSwapchain(const GfxSwapchainDesc& desc, GfxQueue* pPresentQueue)
	{
		auto* pSwapchain = Allocator::Create<D3D12Swapchain>(CurrLoc());

		pSwapchain->m_ownerDevice = this;
		pSwapchain->m_device = this;
		pSwapchain->m_desc = desc;

		BOOL allowTearing = FALSE;

		if (SUCCEEDED(m_factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING,
			&allowTearing, sizeof(allowTearing))))
		{
			pSwapchain->m_allowTearing = desc.bAllowTearing && allowTearing == TRUE;
		}

		DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};

		swapchainDesc.Width = desc.width;
		swapchainDesc.Height = desc.height;
		swapchainDesc.Format = Helpers::ToDXGIFormat(desc.format);
		swapchainDesc.SampleDesc = { 1, 0 };
		swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapchainDesc.BufferCount = desc.imageCount;
		swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
		swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		swapchainDesc.Flags = pSwapchain->m_allowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

		ID3D12CommandQueue* pCommandQueue = static_cast<D3D12Queue*>(pPresentQueue)->Handle();

		IDXGISwapChain1* pSwapchain1 = nullptr;

		HRESULT hr = m_factory->CreateSwapChainForHwnd(pCommandQueue, (HWND)desc.pWindowHandle,
			&swapchainDesc, nullptr, nullptr, &pSwapchain1);
		CHECK_HR(hr, "IDXGISwapChain1 - CreateSwapChainForHwnd");

		hr = pSwapchain1->QueryInterface(IID_PPV_ARGS(&pSwapchain->m_swapchain));
		CHECK_HR(hr, "IDXGISwapChain4 - QueryInterface");
		pSwapchain1->Release();

		pSwapchain->AcquireBackbuffers();

		return pSwapchain;
	}

	GfxDescriptorHeap* D3D12Device::GetDescriptorHeap(GfxDescriptorHeapType type)
	{
		switch (type)
		{
		case GfxDescriptorHeapType::Resource: return m_resourceHeap;
		case GfxDescriptorHeapType::Sampler: return m_samplerHeap;
		case GfxDescriptorHeapType::Color: return m_colorHeap;
		case GfxDescriptorHeapType::Depth: return m_depthHeap;
		}

		Terminal::Error("D3D12Device", "Unknown descriptor heap type {}", u32(type));
		return nullptr;
	}

	GfxAdapterInfo D3D12Device::GetAdapterInfo() const
	{
		DXGI_ADAPTER_DESC3 adapterDesc = {};

		m_adapter->GetDesc3(&adapterDesc);

		GfxAdapterInfo info = {};

		info.dedicatedVideoMemory = adapterDesc.DedicatedVideoMemory;
		info.sharedSystemMemory = adapterDesc.SharedSystemMemory;
		info.vendorId = adapterDesc.VendorId;
		info.deviceId = adapterDesc.DeviceId;

		for (u32 i = 0; i < 127 && adapterDesc.Description[i] != 0; i++)
			info.name[i] = char(adapterDesc.Description[i]);

		return info;
	}

	GfxMemoryStats D3D12Device::QueryMemoryStats()
	{
		DXGI_QUERY_VIDEO_MEMORY_INFO memoryInfo = {};

		m_adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &memoryInfo);

		GfxMemoryStats stats = {};

		stats.budget = memoryInfo.Budget;
		stats.usage = memoryInfo.CurrentUsage;
		return stats;
	}

	D3D12Texture* D3D12Device::CreateBackbufferTexture(ID3D12Resource* pResource, u32 width, u32 height, DXGI_FORMAT format)
	{
		auto* pTexture = Allocator::Create<D3D12Texture>(CurrLoc());

		pTexture->m_ownerDevice = this;
		pTexture->m_resource = pResource;
		pTexture->m_dxgiFormat = format;
		pTexture->m_state = GfxResourceState::Present;
		pTexture->m_isBackbuffer = true;
		pTexture->m_desc.width = width;
		pTexture->m_desc.height = height;
		pTexture->m_desc.usage = GfxTextureUsage::RenderTarget;

		CreateTextureRTV(pTexture);

		return pTexture;
	}

	void D3D12Device::DestroyBackbufferTexture(D3D12Texture* pTexture)
	{
		Allocator::Delete(pTexture);
	}

	D3D12DescriptorHeap* D3D12Device::CreateDescriptorHeap(GfxDescriptorHeapType type, u32 capacity, b8 shaderVisible)
	{
		auto* pHeap = Allocator::Create<D3D12DescriptorHeap>(CurrLoc());

		pHeap->m_ownerDevice = this;
		pHeap->m_desc = { type, capacity, shaderVisible };

		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};

		heapDesc.Type = Helpers::ToDescriptorHeapType(type);
		heapDesc.NumDescriptors = capacity;
		heapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
			: D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		HRESULT hr = m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&pHeap->m_heap));
		CHECK_HR(hr, "ID3D12DescriptorHeap - CreateDescriptorHeap");

		pHeap->m_descriptorSize = m_device->GetDescriptorHandleIncrementSize(heapDesc.Type);
		pHeap->m_cpuStart = pHeap->m_heap->GetCPUDescriptorHandleForHeapStart();
		pHeap->m_gpuStart = shaderVisible ? pHeap->m_heap->GetGPUDescriptorHandleForHeapStart()
			: D3D12_GPU_DESCRIPTOR_HANDLE{};

		return pHeap;
	}

	void D3D12Device::CreateTextureSRV(D3D12Texture* pTex)
	{
		const u32 index = m_resourceHeap->Allocate();
		const b8 isArray = HasFlag(pTex->m_desc.typeFlags, GfxTextureTypeFlags::Array);
		const b8 isCube = HasFlag(pTex->m_desc.typeFlags, GfxTextureTypeFlags::Cube);

		D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc = {};

		viewDesc.Format = Helpers::ToDepthSRVFormat(pTex->m_dxgiFormat);
		viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		switch (pTex->m_desc.type)
		{
		case GfxTextureType::Tex1D:
			if (isArray)
			{
				viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
				viewDesc.Texture1DArray.MipLevels = pTex->m_desc.mipLevels;
				viewDesc.Texture1DArray.ArraySize = pTex->m_desc.arraySize;
			}
			else
			{
				viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
				viewDesc.Texture1D.MipLevels = pTex->m_desc.mipLevels;
			}
			break;
		case GfxTextureType::Tex2D:
			if (isCube && pTex->m_desc.arraySize > 6)
			{
				viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
				viewDesc.TextureCubeArray.MipLevels = pTex->m_desc.mipLevels;
				viewDesc.TextureCubeArray.NumCubes = pTex->m_desc.arraySize / 6;
			}
			else if (isCube)
			{
				viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
				viewDesc.TextureCube.MipLevels = pTex->m_desc.mipLevels;
			}
			else if (isArray)
			{
				viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
				viewDesc.Texture2DArray.MipLevels = pTex->m_desc.mipLevels;
				viewDesc.Texture2DArray.ArraySize = pTex->m_desc.arraySize;
			}
			else
			{
				viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				viewDesc.Texture2D.MipLevels = pTex->m_desc.mipLevels;
			}
			break;
		case GfxTextureType::Tex3D:
			viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
			viewDesc.Texture3D.MipLevels = pTex->m_desc.mipLevels;
			break;
		}

		m_device->CreateShaderResourceView(pTex->m_resource, &viewDesc, m_resourceHeap->CpuAt(index));

		pTex->m_shaderView = index;
	}

	void D3D12Device::CreateTextureUAV(D3D12Texture* pTex)
	{
		const u32 index = m_resourceHeap->Allocate();
		const b8 isArray = HasFlag(pTex->m_desc.typeFlags, GfxTextureTypeFlags::Array)
			|| HasFlag(pTex->m_desc.typeFlags, GfxTextureTypeFlags::Cube);

		D3D12_UNORDERED_ACCESS_VIEW_DESC viewDesc = {};

		viewDesc.Format = pTex->m_dxgiFormat;

		switch (pTex->m_desc.type)
		{
		case GfxTextureType::Tex1D:
			viewDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
			break;
		case GfxTextureType::Tex2D:
			if (isArray)
			{
				viewDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
				viewDesc.Texture2DArray.ArraySize = pTex->m_desc.arraySize;
			}
			else
			{
				viewDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
			}
			break;
		case GfxTextureType::Tex3D:
			viewDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
			viewDesc.Texture3D.WSize = pTex->m_desc.depth;
			break;
		}

		m_device->CreateUnorderedAccessView(pTex->m_resource, nullptr, &viewDesc, m_resourceHeap->CpuAt(index));

		pTex->m_accessView = index;
	}

	void D3D12Device::CreateTextureRTV(D3D12Texture* pTex)
	{
		const u32 index = m_colorHeap->Allocate();

		m_device->CreateRenderTargetView(pTex->m_resource, nullptr, m_colorHeap->CpuAt(index));

		pTex->m_targetViewIndex = index;
		pTex->m_rtvHandle = m_colorHeap->CpuAt(index);
	}

	void D3D12Device::CreateTextureDSV(D3D12Texture* pTex)
	{
		const u32 index = m_depthHeap->Allocate();
		const b8 isArray = HasFlag(pTex->m_desc.typeFlags, GfxTextureTypeFlags::Array);

		D3D12_DEPTH_STENCIL_VIEW_DESC viewDesc = {};

		viewDesc.Format = pTex->m_dxgiFormat;

		if (isArray)
		{
			viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
			viewDesc.Texture2DArray.ArraySize = pTex->m_desc.arraySize;
		}
		else
		{
			viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		}

		m_device->CreateDepthStencilView(pTex->m_resource, &viewDesc, m_depthHeap->CpuAt(index));

		pTex->m_depthViewIndex = index;
		pTex->m_dsvHandle = m_depthHeap->CpuAt(index);
	}

	void D3D12Device::CreateBufferSRV(D3D12Buffer* pBuf)
	{
		const u32 index = m_resourceHeap->Allocate();

		D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc = {};

		viewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		if (pBuf->m_desc.stride > 0)
		{
			viewDesc.Format = DXGI_FORMAT_UNKNOWN;
			viewDesc.Buffer.NumElements = u32(pBuf->m_desc.size / pBuf->m_desc.stride);
			viewDesc.Buffer.StructureByteStride = pBuf->m_desc.stride;
		}
		else
		{
			viewDesc.Format = DXGI_FORMAT_R32_TYPELESS;
			viewDesc.Buffer.NumElements = u32(pBuf->m_desc.size / 4);
			viewDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
		}

		m_device->CreateShaderResourceView(pBuf->m_resource, &viewDesc, m_resourceHeap->CpuAt(index));

		pBuf->m_shaderView = index;
	}

	void D3D12Device::CreateBufferUAV(D3D12Buffer* pBuf)
	{
		const u32 index = m_resourceHeap->Allocate();

		D3D12_UNORDERED_ACCESS_VIEW_DESC viewDesc = {};

		viewDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;

		if (pBuf->m_desc.stride > 0)
		{
			viewDesc.Format = DXGI_FORMAT_UNKNOWN;
			viewDesc.Buffer.NumElements = u32(pBuf->m_desc.size / pBuf->m_desc.stride);
			viewDesc.Buffer.StructureByteStride = pBuf->m_desc.stride;
		}
		else
		{
			viewDesc.Format = DXGI_FORMAT_R32_TYPELESS;
			viewDesc.Buffer.NumElements = u32(pBuf->m_desc.size / 4);
			viewDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
		}

		m_device->CreateUnorderedAccessView(pBuf->m_resource, nullptr, &viewDesc, m_resourceHeap->CpuAt(index));

		pBuf->m_accessView = index;
	}

	void D3D12Device::CreateRootSignature()
	{
		constexpr u32 kRootConstantCount = 16;

		D3D12_ROOT_PARAMETER1 params[1] = {};

		params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		params[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		params[0].Constants.ShaderRegister = 0;
		params[0].Constants.RegisterSpace = 0;
		params[0].Constants.Num32BitValues = kRootConstantCount;

		auto makeSampler = [](GfxStaticSampler slot, D3D12_FILTER filter, D3D12_TEXTURE_ADDRESS_MODE addr,
			D3D12_COMPARISON_FUNC comp = D3D12_COMPARISON_FUNC_NEVER,
			D3D12_STATIC_BORDER_COLOR border = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK,
			u32 aniso = 0)
			{
				D3D12_STATIC_SAMPLER_DESC sampDesc = {};

				sampDesc.Filter = filter;
				sampDesc.AddressU = addr;
				sampDesc.AddressV = addr;
				sampDesc.AddressW = addr;
				sampDesc.MipLODBias = 0.0f;
				sampDesc.MaxAnisotropy = aniso;
				sampDesc.ComparisonFunc = comp;
				sampDesc.BorderColor = border;
				sampDesc.MinLOD = 0.0f;
				sampDesc.MaxLOD = D3D12_FLOAT32_MAX;
				sampDesc.ShaderRegister = u32(slot);
				sampDesc.RegisterSpace = 0;
				sampDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
				return sampDesc;
			};

		const D3D12_STATIC_SAMPLER_DESC samplers[] =
		{
			makeSampler(GfxStaticSampler::PointClamp, D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP),
			makeSampler(GfxStaticSampler::PointWrap, D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_WRAP),
			makeSampler(GfxStaticSampler::LinearClamp, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP),
			makeSampler(GfxStaticSampler::LinearWrap, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP),
			makeSampler(GfxStaticSampler::LinearMirror, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_MIRROR),
			makeSampler(GfxStaticSampler::AnisoClamp, D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
				D3D12_COMPARISON_FUNC_NEVER, D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK, 16),
			makeSampler(GfxStaticSampler::AnisoWrap, D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
				D3D12_COMPARISON_FUNC_NEVER, D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK, 16),
			makeSampler(GfxStaticSampler::ShadowCompare, D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_COMPARISON_FUNC_LESS_EQUAL, D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE),
			makeSampler(GfxStaticSampler::RevShadowCompare, D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_COMPARISON_FUNC_GREATER_EQUAL, D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK),
		};

		D3D12_VERSIONED_ROOT_SIGNATURE_DESC vdesc = {};

		vdesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
		vdesc.Desc_1_1.NumParameters = _countof(params);
		vdesc.Desc_1_1.pParameters = params;
		vdesc.Desc_1_1.NumStaticSamplers = _countof(samplers);
		vdesc.Desc_1_1.pStaticSamplers = samplers;
		vdesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED
			| D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED;

		ID3DBlob* pBlob = nullptr;
		ID3DBlob* pError = nullptr;

		HRESULT hr = D3D12SerializeVersionedRootSignature(&vdesc, &pBlob, &pError);
		CHECK_HR(hr, "ID3D12RootSignature - D3D12SerializeVersionedRootSignature");

		hr = m_device->CreateRootSignature(0, pBlob->GetBufferPointer(), pBlob->GetBufferSize(),
			IID_PPV_ARGS(&m_rootSignature));
		CHECK_HR(hr, "ID3D12RootSignature - CreateRootSignature");

		pBlob->Release();
	}

	void D3D12Device::CreateCommandSignatures()
	{
		D3D12_INDIRECT_ARGUMENT_DESC arg = {};
		D3D12_COMMAND_SIGNATURE_DESC sigDesc = {};

		sigDesc.NumArgumentDescs = 1;
		sigDesc.pArgumentDescs = &arg;

		arg.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;
		sigDesc.ByteStride = sizeof(D3D12_DRAW_ARGUMENTS);

		HRESULT hr = m_device->CreateCommandSignature(&sigDesc, nullptr, IID_PPV_ARGS(&m_drawSignature));
		CHECK_HR(hr, "ID3D12CommandSignature - CreateCommandSignature (Draw)");

		arg.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
		sigDesc.ByteStride = sizeof(D3D12_DRAW_INDEXED_ARGUMENTS);

		hr = m_device->CreateCommandSignature(&sigDesc, nullptr, IID_PPV_ARGS(&m_drawIndexedSignature));
		CHECK_HR(hr, "ID3D12CommandSignature - CreateCommandSignature (DrawIndexed)");

		arg.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;
		sigDesc.ByteStride = sizeof(D3D12_DISPATCH_ARGUMENTS);

		hr = m_device->CreateCommandSignature(&sigDesc, nullptr, IID_PPV_ARGS(&m_dispatchSignature));
		CHECK_HR(hr, "ID3D12CommandSignature - CreateCommandSignature (Dispatch)");
	}

#if defined(HORIZON_DEBUG)
	void D3D12Device::CreateTerminalLog()
	{
		if (FAILED(m_device->QueryInterface(IID_PPV_ARGS(&m_infoQueue))))
			return;

		auto cb = [](D3D12_MESSAGE_CATEGORY, D3D12_MESSAGE_SEVERITY sev,
			D3D12_MESSAGE_ID, LPCSTR pDesc, void*)
			{
				Terminal::Log("D3D12", "[{}]: {}", u32(sev), pDesc);
			};

		m_infoQueue->RegisterMessageCallback(cb, D3D12_MESSAGE_CALLBACK_FLAG_NONE, nullptr, &m_callbackCookie);
	}
#endif
}