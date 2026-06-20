#include "D3D12Device.h"

#include <Runtime/D3D12/D3D12Buffer.h>
#include <Runtime/D3D12/D3D12CommandList.h>
#include <Runtime/D3D12/D3D12DescriptorHeap.h>
#include <Runtime/D3D12/D3D12Fence.h>
#include <Runtime/D3D12/D3D12Queue.h>
#include <Runtime/D3D12/D3D12Swapchain.h>
#include <Runtime/D3D12/D3D12Texture.h>

namespace Horizon
{
	GfxDevice* CreateGfxDevice()
	{
		auto* device = Allocator::Create<D3D12Device>(CurrLoc());
		device->Init();
		return device;
	}

	void D3D12Device::Init()
	{
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

		ID3D12Device* base = nullptr;
		hr = D3D12CreateDevice(m_adapter, D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&base));
		CHECK_HR(hr, "ID3D12Device - D3D12CreateDevice");

		hr = base->QueryInterface(IID_PPV_ARGS(&m_device));
		CHECK_HR(hr, "ID3D12Device10 - QueryInterface");
		base->Release();

		D3D12MA::ALLOCATOR_DESC allocDesc = {};
		allocDesc.pDevice = m_device;
		allocDesc.pAdapter = m_adapter;
		D3D12MA::CreateAllocator(&allocDesc, &m_allocator);

#if defined(HORIZON_DEBUG)
		CreateTerminalLog();
#endif

		CreateDescriptorHeap(m_resourceHeap, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1 << 16, true);
		CreateDescriptorHeap(m_colorHeap, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1024, false);
		CreateDescriptorHeap(m_depthHeap, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 256, false);

		CreateRootSignature();
	}

	D3D12Device::~D3D12Device()
	{
		FlushPendingDeletes(kInvalid64);

		if (m_resourceHeap.pHeap)
			m_resourceHeap.pHeap->Release();

		if (m_colorHeap.pHeap)
			m_colorHeap.pHeap->Release();

		if (m_depthHeap.pHeap)
			m_depthHeap.pHeap->Release();

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

	GfxTexture* D3D12Device::CreateTexture(const GfxTextureDesc& desc)
	{
		const DXGI_FORMAT viewFormat = Helpers::ToDXGIFormat(desc.format);
		const b8 isDepth = Helpers::IsDepthFormat(desc.format);
		const b8 isSampled = HasFlag(desc.usage, GfxTextureUsage::Sampled);

		D3D12_RESOURCE_DESC resourceDesc = {};
		resourceDesc.Dimension = desc.type == GfxTextureType::Tex3D ? D3D12_RESOURCE_DIMENSION_TEXTURE3D
			: D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resourceDesc.Width = desc.width;  resourceDesc.Height = desc.height;
		resourceDesc.DepthOrArraySize = u16(desc.depth);
		resourceDesc.MipLevels = u16(desc.mipLevels);
		resourceDesc.Format = (isDepth && isSampled) ? Helpers::ToTypelessFormat(viewFormat) : viewFormat;
		resourceDesc.SampleDesc = { 1, 0 };
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

		if (HasFlag(desc.usage, GfxTextureUsage::RenderTarget))
			resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

		if (HasFlag(desc.usage, GfxTextureUsage::DepthStencil))
			resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		if (HasFlag(desc.usage, GfxTextureUsage::Storage))
			resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

		if (isDepth && !isSampled)
			resourceDesc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;

		D3D12_CLEAR_VALUE clear = {};
		clear.Format = viewFormat;
		if (isDepth)
		{
			clear.DepthStencil = { 1.0f, 0 };
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

		auto* tex = Allocator::Create<D3D12Texture>(CurrLoc());
		tex->m_ownerDevice = this;

		HRESULT bResult = m_allocator->CreateResource(&allocDesc, &resourceDesc, D3D12_RESOURCE_STATE_COMMON,
			wantsClear ? &clear : nullptr, &tex->m_allocation, IID_PPV_ARGS(&tex->m_resource));
		CHECK_REASON(bResult, "ID3D12Resource - CreateResouce");

		if (FAILED(bResult))
		{
			Allocator::Delete(tex);
			return {};
		}

		tex->m_desc = desc;
		tex->m_state = GfxResourceState::Common;
		tex->m_dxgiFormat = viewFormat;

		if (isSampled)
			CreateTextureSRV(tex);
		if (HasFlag(desc.usage, GfxTextureUsage::Storage))
			CreateTextureUAV(tex);
		if (HasFlag(desc.usage, GfxTextureUsage::RenderTarget))
			CreateTextureRTV(tex);
		if (HasFlag(desc.usage, GfxTextureUsage::DepthStencil))
			CreateTextureDSV(tex);

		return tex;
	}

	GfxBuffer* D3D12Device::CreateBuffer(const GfxBufferDesc& desc)
	{
		usize size = desc.size;
		if (HasFlag(desc.usage, GfxBufferUsage::Constant))
			size = (size + 255) & ~usize(255);

		D3D12_RESOURCE_DESC rd = {};
		rd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		rd.Width = size;  rd.Height = 1;
		rd.DepthOrArraySize = 1;  rd.MipLevels = 1;
		rd.Format = DXGI_FORMAT_UNKNOWN;
		rd.SampleDesc = { 1, 0 };
		rd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		rd.Flags = HasFlag(desc.usage, GfxBufferUsage::Storage)
			? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;

		D3D12MA::ALLOCATION_DESC ad = {};
		ad.HeapType =
			desc.memory == GfxMemoryType::HostVisible ? D3D12_HEAP_TYPE_UPLOAD :
			desc.memory == GfxMemoryType::CPU ? D3D12_HEAP_TYPE_READBACK :
			D3D12_HEAP_TYPE_DEFAULT;

		const D3D12_RESOURCE_STATES initialState =
			desc.memory == GfxMemoryType::HostVisible ? D3D12_RESOURCE_STATE_GENERIC_READ :
			desc.memory == GfxMemoryType::CPU ? D3D12_RESOURCE_STATE_COPY_DEST :
			D3D12_RESOURCE_STATE_COMMON;

		auto* buf = Allocator::Create<D3D12Buffer>(CurrLoc());
		buf->m_ownerDevice = this;

		HRESULT hr = m_allocator->CreateResource(&ad, &rd, initialState, nullptr,
			&buf->m_allocation, IID_PPV_ARGS(&buf->m_resource));
		CHECK_REASON(hr, "ID3D12Resource - CreateResource (Buffer)");
		if (FAILED(hr)) 
		{ 
			Allocator::Delete(buf);
			return {}; 
		}

		buf->m_desc = desc;
		buf->m_gpuAddress = buf->m_resource->GetGPUVirtualAddress();

		if (desc.memory != GfxMemoryType::GPU)
		{
			D3D12_RANGE noRead = { 0, 0 };
			buf->m_resource->Map(0, &noRead, &buf->m_mapped);
		}

		const b8 shaderRead = HasFlag(desc.usage, GfxBufferUsage::Vertex)
			|| HasFlag(desc.usage, GfxBufferUsage::Index)
			|| HasFlag(desc.usage, GfxBufferUsage::Storage);

		if (shaderRead && desc.memory == GfxMemoryType::GPU)
			CreateBufferSRV(buf);
		if (HasFlag(desc.usage, GfxBufferUsage::Storage))
			CreateBufferUAV(buf);

		return buf;
	}

	GfxQueue* D3D12Device::CreateQueue(GfxQueueType type)
	{
		auto queue = Allocator::Create<D3D12Queue>(CurrLoc());
		queue->m_type = type;
		D3D12_COMMAND_QUEUE_DESC qd = {};
		qd.Type = Helpers::ToListType(type);
		m_device->CreateCommandQueue(&qd, IID_PPV_ARGS(&queue->m_queue));
		return queue;
	}

	GfxFence* D3D12Device::CreateFence()
	{
		auto fence = Allocator::Create<D3D12Fence>(CurrLoc());
		m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence->m_fence));
		return fence;
	}

	GfxCommandList* D3D12Device::CreateCommandList(GfxQueueType type)
	{
		auto cmd = Allocator::Create<D3D12CommandList>(CurrLoc());
		cmd->m_device = this;

		const D3D12_COMMAND_LIST_TYPE lt = Helpers::ToListType(type);
		cmd->m_type = lt;
		m_device->CreateCommandAllocator(lt, IID_PPV_ARGS(&cmd->m_allocator));
		m_device->CreateCommandList1(0, lt, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&cmd->m_list));

		return cmd;
	}

	GfxSwapchain* D3D12Device::CreateSwapchain(const GfxSwapchainDesc& desc, GfxQueue* presentQueue)
	{
		auto swapchain = Allocator::Create<D3D12Swapchain>(CurrLoc());
		swapchain->m_device = this;
		swapchain->m_width = desc.width;
		swapchain->m_height = desc.height;
		swapchain->m_vSync = desc.vSync;
		swapchain->m_imageCount = desc.imageCount;

		BOOL allowTearing = FALSE;
		if (SUCCEEDED(m_factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING,
			&allowTearing, sizeof(allowTearing))))
			swapchain->m_allowTearing = (allowTearing == TRUE);

		DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
		swapchainDesc.Width = desc.width;
		swapchainDesc.Height = desc.height;
		swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapchainDesc.SampleDesc = { 1, 0 };
		swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapchainDesc.BufferCount = desc.imageCount;
		swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
		swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		swapchainDesc.Flags = swapchain->m_allowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

		ID3D12CommandQueue* commandQueue = static_cast<D3D12Queue*>(presentQueue)->Handle();

		IDXGISwapChain1* swapchain1 = nullptr;
		HRESULT hr = m_factory->CreateSwapChainForHwnd(commandQueue, (HWND)desc.pWindowHandle,
			&swapchainDesc, nullptr, nullptr, &swapchain1);
		CHECK_HR(hr, "IDXGISwapChain1 - CreateSwapChainForHwnd");

		hr = swapchain1->QueryInterface(IID_PPV_ARGS(&swapchain->m_swapchain));
		CHECK_HR(hr, "IDXGISwapChain4 - QueryInterface");
		swapchain1->Release();

		swapchain->AcquireBackbuffers();

		return swapchain;
	}

	D3D12Texture* D3D12Device::CreateBackbufferTexture(ID3D12Resource* resource, u32 width, u32 height, DXGI_FORMAT format)
	{
		auto* texture = Allocator::Create<D3D12Texture>(CurrLoc());
		texture->m_ownerDevice = this;
		texture->m_resource = resource;
		texture->m_dxgiFormat = format;
		texture->m_state = GfxResourceState::Present;
		texture->m_isBackbuffer = true;
		texture->m_desc.width = width;
		texture->m_desc.height = height;

		CreateTextureRTV(texture);

		return texture;
	}

	void D3D12Device::DestroyBackbufferTexture(D3D12Texture* texture)
	{
		Allocator::Delete(texture);
	}

	void D3D12Device::CreateTextureSRV(D3D12Texture* tex)
	{
		u32 index = m_resourceHeap.Allocate();

		D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
		viewDesc.Format = Helpers::ToDepthSRVFormat(tex->m_dxgiFormat);
		viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		switch (tex->m_desc.type)
		{
		case GfxTextureType::Tex2D:
			viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			viewDesc.Texture2D.MipLevels = tex->m_desc.mipLevels;
			break;
		case GfxTextureType::Tex2DArray:
			viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
			viewDesc.Texture2DArray.MipLevels = tex->m_desc.mipLevels;
			viewDesc.Texture2DArray.ArraySize = tex->m_desc.depth;
			break;
		case GfxTextureType::Tex3D:
			viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
			viewDesc.Texture3D.MipLevels = tex->m_desc.mipLevels;
			break;
		}

		m_device->CreateShaderResourceView(tex->m_resource, &viewDesc, m_resourceHeap.CpuAt(index));

		tex->m_shaderView = index;
	}

	void D3D12Device::CreateTextureUAV(D3D12Texture* tex)
	{
		u32 index = m_resourceHeap.Allocate();

		D3D12_UNORDERED_ACCESS_VIEW_DESC viewDesc = {};
		viewDesc.Format = tex->m_dxgiFormat;
		viewDesc.ViewDimension = tex->m_desc.type == GfxTextureType::Tex3D
			? D3D12_UAV_DIMENSION_TEXTURE3D : D3D12_UAV_DIMENSION_TEXTURE2D;

		if (tex->m_desc.type == GfxTextureType::Tex3D)
			viewDesc.Texture3D.WSize = tex->m_desc.depth;

		m_device->CreateUnorderedAccessView(tex->m_resource, nullptr, &viewDesc, m_resourceHeap.CpuAt(index));

		tex->m_accessView = index;
	}

	void D3D12Device::CreateTextureRTV(D3D12Texture* tex)
	{
		u32 index = m_colorHeap.Allocate();

		m_device->CreateRenderTargetView(tex->m_resource, nullptr, m_colorHeap.CpuAt(index));

		tex->m_targetViewIndex = index;
		tex->m_rtvHandle = m_colorHeap.CpuAt(index);
	}

	void D3D12Device::CreateTextureDSV(D3D12Texture* tex)
	{
		u32 index = m_depthHeap.Allocate();

		D3D12_DEPTH_STENCIL_VIEW_DESC viewDesc = {};
		viewDesc.Format = tex->m_dxgiFormat;
		viewDesc.ViewDimension = tex->m_desc.type == GfxTextureType::Tex2DArray
			? D3D12_DSV_DIMENSION_TEXTURE2DARRAY : D3D12_DSV_DIMENSION_TEXTURE2D;

		if (tex->m_desc.type == GfxTextureType::Tex2DArray)
			viewDesc.Texture2DArray.ArraySize = tex->m_desc.depth;

		m_device->CreateDepthStencilView(tex->m_resource, &viewDesc, m_depthHeap.CpuAt(index));

		tex->m_depthViewIndex = index;
		tex->m_dsvHandle = m_depthHeap.CpuAt(index);
	}

	void D3D12Device::CreateBufferSRV(D3D12Buffer* buf)
	{
		u32 index = m_resourceHeap.Allocate();

		D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
		viewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		if (buf->m_desc.stride > 0)
		{
			viewDesc.Format = DXGI_FORMAT_UNKNOWN;
			viewDesc.Buffer.NumElements = u32(buf->m_desc.size / buf->m_desc.stride);
			viewDesc.Buffer.StructureByteStride = buf->m_desc.stride;
		}
		else
		{
			viewDesc.Format = DXGI_FORMAT_R32_TYPELESS;
			viewDesc.Buffer.NumElements = u32(buf->m_desc.size / 4);
			viewDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
		}
		m_device->CreateShaderResourceView(buf->m_resource, &viewDesc, m_resourceHeap.CpuAt(index));
		buf->m_shaderView = index;
	}

	void D3D12Device::CreateBufferUAV(D3D12Buffer* buf)
	{
		u32 index = m_resourceHeap.Allocate();

		D3D12_UNORDERED_ACCESS_VIEW_DESC viewDesc = {};
		viewDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		if (buf->m_desc.stride > 0)
		{
			viewDesc.Format = DXGI_FORMAT_UNKNOWN;
			viewDesc.Buffer.NumElements = u32(buf->m_desc.size / buf->m_desc.stride);
			viewDesc.Buffer.StructureByteStride = buf->m_desc.stride;
		}
		else
		{
			viewDesc.Format = DXGI_FORMAT_R32_TYPELESS;
			viewDesc.Buffer.NumElements = u32(buf->m_desc.size / 4);
			viewDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
		}
		m_device->CreateUnorderedAccessView(buf->m_resource, nullptr, &viewDesc, m_resourceHeap.CpuAt(index));
		buf->m_accessView = index;
	}

	void D3D12Device::CreateDescriptorHeap(D3D12DescriptorHeap& out, D3D12_DESCRIPTOR_HEAP_TYPE type, u32 capacity, b8 shaderVisible)
	{
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.Type = type;
		heapDesc.NumDescriptors = capacity;
		heapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
			: D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		HRESULT hr = m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&out.pHeap));
		CHECK_HR(hr, "CreateDescriptorHeap");

		out.descriptorSize = m_device->GetDescriptorHandleIncrementSize(type);
		out.capacity = capacity;
		out.cpuStart = out.pHeap->GetCPUDescriptorHandleForHeapStart();
		out.gpuStart = shaderVisible ? out.pHeap->GetGPUDescriptorHandleForHeapStart()
			: D3D12_GPU_DESCRIPTOR_HANDLE{};
		out.top = 0;
		out.freeList.clear();
	}

#if defined(HORIZON_DEBUG)
	void D3D12Device::CreateTerminalLog()
	{
		if (FAILED(m_device->QueryInterface(IID_PPV_ARGS(&m_infoQueue))))
			return;

		auto cb = [](D3D12_MESSAGE_CATEGORY, D3D12_MESSAGE_SEVERITY sev,
			D3D12_MESSAGE_ID, LPCSTR desc, void*)
			{
				Terminal::Log("D3D12", "[{}]: {}", u32(sev), desc);
			};
		m_infoQueue->RegisterMessageCallback(cb, D3D12_MESSAGE_CALLBACK_FLAG_NONE, nullptr, &m_callbackCookie);
	}
#endif
}