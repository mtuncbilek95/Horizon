#include "D3D12Device.h"

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Definitions/Allocator.h>
#include <Runtime/Log/Terminal.h>

#include <Runtime/RHI/Buffer/GfxBufferArenaDesc.h>
#include <Runtime/RHI/Sampler/GfxStaticSampler.h>
#include <Runtime/RHI/Shader/GfxShaderDesc.h>
#include <Runtime/RHI/Swapchain/GfxSwapchainDesc.h>
#include <Runtime/RHI/Texture/GfxTextureDesc.h>
#include <Runtime/RHI/Upload/GfxUploadRingDesc.h>

#include <Runtime/D3D12/D3D12Buffer.h>
#include <Runtime/D3D12/D3D12BufferArena.h>
#include <Runtime/D3D12/D3D12CommandList.h>
#include <Runtime/D3D12/D3D12DescriptorHeap.h>
#include <Runtime/D3D12/D3D12Fence.h>
#include <Runtime/D3D12/D3D12Queue.h>
#include <Runtime/D3D12/D3D12Shader.h>
#include <Runtime/D3D12/D3D12Swapchain.h>
#include <Runtime/D3D12/D3D12Texture.h>
#include <Runtime/D3D12/D3D12UploadRing.h>

#include <imgui.h>
#include <backends/imgui_impl_dx12.h>

namespace Horizon::RHI
{
	namespace
	{
		void ImGuiAlloc(ImGui_ImplDX12_InitInfo* pInfo, D3D12_CPU_DESCRIPTOR_HANDLE* pOutCpu, D3D12_GPU_DESCRIPTOR_HANDLE* pOutGpu)
		{
			auto* pHeap = static_cast<D3D12DescriptorHeap*>(pInfo->UserData);
			const u32 index = pHeap->Allocate();

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
			auto* pHeap = static_cast<D3D12DescriptorHeap*>(pInfo->UserData);
			pHeap->Free(pHeap->IndexOf(cpu));
		}
	}

	GfxDevice* CreateDevice(const GfxDeviceDesc& desc)
	{
		auto* pDevice = Memory::Allocator::Create<D3D12Device>(Memory::CurrLoc());
		pDevice->Init(desc);

		return pDevice;
	}

	D3D12Device::~D3D12Device()
	{
		WaitIdle();

		if (m_idleFence)
			m_idleFence->Release();

		if (m_dispatchSignature)
			m_dispatchSignature->Release();

		if (m_drawIndexedSignature)
			m_drawIndexedSignature->Release();

		if (m_drawSignature)
			m_drawSignature->Release();

		if (m_rootSignature)
			m_rootSignature->Release();

		if (m_allocator)
			m_allocator->Release();

		if (m_infoQueue)
		{
			m_infoQueue->UnregisterMessageCallback(m_callbackCookie);
			m_infoQueue->Release();
		}

		if (m_device)
			m_device->Release();

		if (m_adapter)
			m_adapter->Release();

		if (m_factory)
			m_factory->Release();

		if (m_debug)
			m_debug->Release();
	}

	void D3D12Device::Init(const GfxDeviceDesc& desc)
	{
		u32 factoryFlags = 0;
		if (desc.enableDebugLayer && SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&m_debug))))
		{
			m_debug->EnableDebugLayer();
			factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;

			if (desc.enableGpuValidation)
				m_debug->SetEnableGPUBasedValidation(TRUE);

			if (desc.synchronizedCommandValidation)
				m_debug->SetEnableSynchronizedCommandQueueValidation(TRUE);
		}

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

		hr = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_idleFence));
		CHECK_HR(hr, "ID3D12Fence - CreateFence (idle)");

		CreateTerminalLog();
		CreateRootSignature();
		CreateCommandSignatures();
	}

	void D3D12Device::InitializeImGui(u32 maxFrames, GfxQueue* pQueue, GfxDescriptorHeap* pHeap, GfxTextureFormat colorFormat)
	{
		auto* pD3DQueue = static_cast<D3D12Queue*>(pQueue);
		auto* pD3DHeap = static_cast<D3D12DescriptorHeap*>(pHeap);

		ImGui_ImplDX12_InitInfo info = {};

		info.Device = m_device;
		info.CommandQueue = pD3DQueue->Handle();
		info.NumFramesInFlight = i32(maxFrames);
		info.RTVFormat = Helpers::ToSwapchainFormat(Helpers::ToFormat(colorFormat));
		info.DSVFormat = DXGI_FORMAT_UNKNOWN;
		info.SrvDescriptorHeap = pD3DHeap->Handle();
		info.SrvDescriptorAllocFn = &ImGuiAlloc;
		info.SrvDescriptorFreeFn = &ImGuiFree;
		info.UserData = pHeap;

		ImGui_ImplDX12_Init(&info);
	}

	void D3D12Device::NewFrameImGui()
	{
		ImGui_ImplDX12_NewFrame();
	}

	void D3D12Device::ShutdownImGui()
	{
		ImGui_ImplDX12_Shutdown();
	}

	GfxDescriptorHeap* D3D12Device::CreateDescriptorHeap(const GfxDescriptorHeapDesc& desc)
	{
		auto* pHeap = Memory::Allocator::Create<D3D12DescriptorHeap>(Memory::CurrLoc());

		pHeap->m_ownerDevice = this;
		pHeap->m_desc = desc;

		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};

		heapDesc.Type = Helpers::ToDescriptorHeapType(desc.type);
		heapDesc.NumDescriptors = desc.capacity;
		heapDesc.Flags = desc.shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
			: D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		HRESULT hr = m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&pHeap->m_heap));
		CHECK_HR(hr, "ID3D12DescriptorHeap - CreateDescriptorHeap");

		pHeap->m_descriptorSize = m_device->GetDescriptorHandleIncrementSize(heapDesc.Type);
		pHeap->m_cpuStart = pHeap->m_heap->GetCPUDescriptorHandleForHeapStart();
		pHeap->m_gpuStart = desc.shaderVisible ? pHeap->m_heap->GetGPUDescriptorHandleForHeapStart()
			: D3D12_GPU_DESCRIPTOR_HANDLE{};

		return pHeap;
	}

	GfxSwapchain* D3D12Device::CreateSwapchain(const GfxSwapchainDesc& desc, GfxQueue* pPresentQueue)
	{
		if (desc.imageCount > kMaxSwapchainImages)
		{
			Terminal::Error(StringOps::GetName(this), "Swapchain image count {} exceeds the {} limit",
				desc.imageCount, kMaxSwapchainImages);
			return nullptr;
		}

		if (!desc.pColorHeap)
		{
			Terminal::Error(StringOps::GetName(this), "Swapchain needs a color descriptor heap to build its render targets");
			return nullptr;
		}

		auto* pQueue = static_cast<D3D12Queue*>(pPresentQueue);
		auto* pSwapchain = Memory::Allocator::Create<D3D12Swapchain>(Memory::CurrLoc());

		pSwapchain->m_ownerDevice = this;
		pSwapchain->m_device = this;
		pSwapchain->m_desc = desc;
		pSwapchain->m_syncInterval = Helpers::ToSyncInterval(desc.presentMode);
		pSwapchain->m_presentFlags = Helpers::ToPresentFlags(desc.presentMode);

		DXGI_SWAP_CHAIN_DESC1 chainDesc = {};

		chainDesc.Width = desc.width;
		chainDesc.Height = desc.height;
		chainDesc.Format = Helpers::ToSwapchainFormat(Helpers::ToFormat(desc.format));
		chainDesc.SampleDesc = { 1, 0 };
		chainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		chainDesc.BufferCount = desc.imageCount;
		chainDesc.Scaling = DXGI_SCALING_NONE;
		chainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		chainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		chainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

		const HWND window = HWND(desc.pWindowHandle);
		IDXGISwapChain1* pBase = nullptr;

		HRESULT hr = m_factory->CreateSwapChainForHwnd(pQueue->Handle(), window, &chainDesc, nullptr, nullptr, &pBase);
		CHECK_HR(hr, "IDXGISwapChain1 - CreateSwapChainForHwnd");

		hr = pBase->QueryInterface(IID_PPV_ARGS(&pSwapchain->m_swapchain));
		CHECK_HR(hr, "IDXGISwapChain4 - QueryInterface");
		pBase->Release();

		m_factory->MakeWindowAssociation(window, DXGI_MWA_NO_ALT_ENTER);

		pSwapchain->AcquireImages();
		pSwapchain->m_imageIndex = pSwapchain->m_swapchain->GetCurrentBackBufferIndex();

		return pSwapchain;
	}

	GfxTexture* D3D12Device::CreateTexture(const GfxTextureDesc& desc)
	{
		auto* pTexture = Memory::Allocator::Create<D3D12Texture>(Memory::CurrLoc());

		pTexture->m_ownerDevice = this;
		pTexture->m_desc = desc;
		pTexture->m_format = Helpers::ToFormat(desc.format);

		const b8 bIsDepth = Helpers::IsDepthFormat(pTexture->m_format);
		const b8 bSampled = HasFlag(desc.usage, GfxTextureUsage::Sampled);

		D3D12_RESOURCE_DESC resourceDesc = {};

		resourceDesc.Dimension = Helpers::ToResourceDimension(desc.type);
		resourceDesc.Width = desc.width;
		resourceDesc.Height = desc.height;
		resourceDesc.MipLevels = u16(desc.mipLevels);
		resourceDesc.SampleDesc = { Helpers::ToSampleCount(desc.sampleCount), 0 };
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resourceDesc.Flags = Helpers::ToResourceFlags(desc.usage);

		if (desc.type == GfxTextureType::Tex3D)
			resourceDesc.DepthOrArraySize = u16(desc.depth);
		else
			resourceDesc.DepthOrArraySize = u16(desc.isCube ? desc.arraySize * 6 : desc.arraySize);

		resourceDesc.Format = bIsDepth && bSampled
			? Helpers::ToTypelessFormat(pTexture->m_format) : pTexture->m_format;

		D3D12_CLEAR_VALUE clearValue = {};

		clearValue.Format = pTexture->m_format;

		if (bIsDepth)
		{
			clearValue.DepthStencil.Depth = 1.0f;
			clearValue.DepthStencil.Stencil = 0;
		}
		else
		{
			clearValue.Color[0] = desc.clearColor.r;
			clearValue.Color[1] = desc.clearColor.g;
			clearValue.Color[2] = desc.clearColor.b;
			clearValue.Color[3] = desc.clearColor.a;
		}

		const b8 bHasClear = HasFlag(desc.usage, GfxTextureUsage::RenderTarget)
			|| HasFlag(desc.usage, GfxTextureUsage::DepthStencil);

		D3D12_RESOURCE_STATES initialState = bIsDepth
			? D3D12_RESOURCE_STATE_DEPTH_WRITE : D3D12_RESOURCE_STATE_COMMON;

		D3D12MA::ALLOCATION_DESC allocDesc = {};

		allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

		HRESULT hr = m_allocator->CreateResource(&allocDesc, &resourceDesc, initialState,
			bHasClear ? &clearValue : nullptr, &pTexture->m_allocation, IID_PPV_ARGS(&pTexture->m_resource));
		CHECK_REASON(hr, "ID3D12Resource - CreateResource (Texture)");

		if (FAILED(hr))
		{
			Memory::Allocator::Delete(pTexture);
			return nullptr;
		}

		return pTexture;
	}

	GfxBuffer* D3D12Device::CreateBuffer(const GfxBufferDesc& desc)
	{
		auto* pBuffer = Memory::Allocator::Create<D3D12Buffer>(Memory::CurrLoc());

		pBuffer->m_ownerDevice = this;
		pBuffer->m_desc = desc;

		D3D12_RESOURCE_DESC resourceDesc = {};

		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Width = desc.size;
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		resourceDesc.SampleDesc = { 1, 0 };
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resourceDesc.Flags = Helpers::ToResourceFlags(desc.usage);

		D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COMMON;

		if (desc.memory == GfxMemoryType::Upload)
			initialState = D3D12_RESOURCE_STATE_GENERIC_READ;

		if (desc.memory == GfxMemoryType::Readback)
			initialState = D3D12_RESOURCE_STATE_COPY_DEST;

		D3D12MA::ALLOCATION_DESC allocDesc = {};

		allocDesc.HeapType = Helpers::ToHeapType(desc.memory);

		HRESULT hr = m_allocator->CreateResource(&allocDesc, &resourceDesc, initialState, nullptr,
			&pBuffer->m_allocation, IID_PPV_ARGS(&pBuffer->m_resource));
		CHECK_REASON(hr, "ID3D12Resource - CreateResource (Buffer)");

		if (FAILED(hr))
		{
			Memory::Allocator::Delete(pBuffer);
			return nullptr;
		}

		pBuffer->m_deviceAddress = pBuffer->m_resource->GetGPUVirtualAddress();

		return pBuffer;
	}

	GfxBufferArena* D3D12Device::CreateBufferArena(const GfxBufferArenaDesc& desc)
	{
		GfxBufferDesc bufferDesc = {};

		bufferDesc.usage = desc.usage;
		bufferDesc.memory = desc.memory;
		bufferDesc.size = desc.capacity;

		GfxBuffer* pBuffer = CreateBuffer(bufferDesc);

		if (!pBuffer)
		{
			Terminal::Error(StringOps::GetName(this), "Arena backing buffer of {} bytes could not be created", desc.capacity);
			return nullptr;
		}

		auto* pArena = Memory::Allocator::Create<D3D12BufferArena>(Memory::CurrLoc());

		pArena->m_ownerDevice = this;
		pArena->m_desc = desc;
		pArena->m_buffer = pBuffer;

		D3D12MA::VIRTUAL_BLOCK_DESC blockDesc = {};

		blockDesc.Size = desc.capacity;

		HRESULT hr = D3D12MA::CreateVirtualBlock(&blockDesc, &pArena->m_block);
		CHECK_HR(hr, "D3D12MA::VirtualBlock - CreateVirtualBlock");

		return pArena;
	}

	GfxUploadRing* D3D12Device::CreateUploadRing(const GfxUploadRingDesc& desc)
	{
		GfxBufferDesc bufferDesc = {};

		bufferDesc.usage = GfxBufferUsage::TransferSrc;
		bufferDesc.memory = GfxMemoryType::Upload;
		bufferDesc.size = desc.capacity;

		GfxBuffer* pBuffer = CreateBuffer(bufferDesc);

		if (!pBuffer)
		{
			Terminal::Error(StringOps::GetName(this), "Upload ring buffer of {} bytes could not be created", desc.capacity);
			return nullptr;
		}

		auto* pRing = Memory::Allocator::Create<D3D12UploadRing>(Memory::CurrLoc());

		pRing->m_ownerDevice = this;
		pRing->m_desc = desc;
		pRing->m_buffer = pBuffer;
		pRing->m_mapped = static_cast<u8*>(pBuffer->Map());

		if (!pRing->m_mapped)
		{
			Memory::Allocator::Delete(pRing);
			return nullptr;
		}

		return pRing;
	}

	GfxShader* D3D12Device::CreateShader(const GfxShaderDesc& desc)
	{
		if (!desc.pByteCode || desc.byteCodeSize == 0)
		{
			Terminal::Error(StringOps::GetName(this), "Shader stage {} was given empty bytecode", u32(desc.stage));
			return nullptr;
		}

		auto* pShader = Memory::Allocator::Create<D3D12Shader>(Memory::CurrLoc());

		pShader->m_ownerDevice = this;
		pShader->m_stage = desc.stage;
		pShader->m_byteCodeSize = desc.byteCodeSize;

		pShader->m_byteCode.Resize(desc.byteCodeSize);
		memcpy(pShader->m_byteCode.GetData(), desc.pByteCode, desc.byteCodeSize);

		return pShader;
	}

	GfxCommandList* D3D12Device::CreateCommandList(GfxQueueType type)
	{
		auto* pCmd = Memory::Allocator::Create<D3D12CommandList>(Memory::CurrLoc());

		pCmd->m_ownerDevice = this;
		pCmd->m_device = this;
		pCmd->m_queueType = type;
		pCmd->m_type = Helpers::ToCommandListType(type);

		HRESULT hr = m_device->CreateCommandAllocator(pCmd->m_type, IID_PPV_ARGS(&pCmd->m_allocator));
		CHECK_HR(hr, "ID3D12CommandAllocator - CreateCommandAllocator");

		hr = m_device->CreateCommandList1(0, pCmd->m_type, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&pCmd->m_list));
		CHECK_HR(hr, "ID3D12GraphicsCommandList6 - CreateCommandList1");

		return pCmd;
	}

	GfxQueue* D3D12Device::CreateQueue(GfxQueueType type)
	{
		auto* pQueue = Memory::Allocator::Create<D3D12Queue>(Memory::CurrLoc());

		pQueue->m_ownerDevice = this;
		pQueue->m_type = type;

		D3D12_COMMAND_QUEUE_DESC queueDesc = {};

		queueDesc.Type = Helpers::ToCommandListType(type);
		queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

		HRESULT hr = m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&pQueue->m_queue));
		CHECK_HR(hr, "ID3D12CommandQueue - CreateCommandQueue");

		m_queues.PushBack(pQueue);

		return pQueue;
	}

	GfxFence* D3D12Device::CreateFence()
	{
		auto* pFence = Memory::Allocator::Create<D3D12Fence>(Memory::CurrLoc());

		pFence->m_ownerDevice = this;

		HRESULT hr = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pFence->m_fence));
		CHECK_HR(hr, "ID3D12Fence - CreateFence");

		return pFence;
	}

	void D3D12Device::ForgetQueue(D3D12Queue* pQueue)
	{
		m_queues.Remove(pQueue);
	}

	void D3D12Device::WaitIdle()
	{
		if (!m_idleFence)
			return;

		for (D3D12Queue* pQueue : m_queues)
		{
			const u64 value = ++m_idleValue;

			pQueue->Handle()->Signal(m_idleFence, value);

			if (m_idleFence->GetCompletedValue() < value)
				m_idleFence->SetEventOnCompletion(value, nullptr);
		}
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
		vdesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED;

		ID3DBlob* pBlob = nullptr;
		ID3DBlob* pError = nullptr;

		HRESULT hr = D3D12SerializeVersionedRootSignature(&vdesc, &pBlob, &pError);
		CHECK_HR(hr, "ID3D12RootSignature - D3D12SerializeVersionedRootSignature");

		hr = m_device->CreateRootSignature(0, pBlob->GetBufferPointer(), pBlob->GetBufferSize(),
			IID_PPV_ARGS(&m_rootSignature));
		CHECK_HR(hr, "ID3D12RootSignature - CreateRootSignature");

		pBlob->Release();

		if (pError)
			pError->Release();
	}

	void D3D12Device::CreateCommandSignatures()
	{
		D3D12_INDIRECT_ARGUMENT_DESC argument = {};
		D3D12_COMMAND_SIGNATURE_DESC signatureDesc = {};

		signatureDesc.NumArgumentDescs = 1;
		signatureDesc.pArgumentDescs = &argument;

		argument.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;
		signatureDesc.ByteStride = sizeof(D3D12_DRAW_ARGUMENTS);

		HRESULT hr = m_device->CreateCommandSignature(&signatureDesc, nullptr, IID_PPV_ARGS(&m_drawSignature));
		CHECK_HR(hr, "ID3D12CommandSignature - CreateCommandSignature (Draw)");

		argument.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
		signatureDesc.ByteStride = sizeof(D3D12_DRAW_INDEXED_ARGUMENTS);

		hr = m_device->CreateCommandSignature(&signatureDesc, nullptr, IID_PPV_ARGS(&m_drawIndexedSignature));
		CHECK_HR(hr, "ID3D12CommandSignature - CreateCommandSignature (DrawIndexed)");

		argument.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;
		signatureDesc.ByteStride = sizeof(D3D12_DISPATCH_ARGUMENTS);

		hr = m_device->CreateCommandSignature(&signatureDesc, nullptr, IID_PPV_ARGS(&m_dispatchSignature));
		CHECK_HR(hr, "ID3D12CommandSignature - CreateCommandSignature (Dispatch)");
	}

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
}
