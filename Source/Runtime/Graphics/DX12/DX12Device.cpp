#include "DX12Device.h"

#include <Runtime/Graphics/RHI/GfxDevice.h>
#include <Runtime/Graphics/RHI/GfxBuffer.h>
#include <Runtime/Graphics/RHI/GfxTexture.h>
#include <Runtime/Graphics/RHI/GfxCommandList.h>
#include <Runtime/Graphics/RHI/GfxPipeline.h>

#define CHECK_HR(hr, what) \
	if(FAILED(hr)) { Horizon::MainLog::Error("{}: {}", what, _com_error(hr).ErrorMessage()); exit(-1); }

namespace Horizon
{
	static void CreateDescHeap(ID3D12Device10* pDevice, DX12DescriptorHeap& outHeap, D3D12_DESCRIPTOR_HEAP_TYPE type, u32 count, b8 shaderVis)
	{
		D3D12_DESCRIPTOR_HEAP_DESC d = {};
		d.Type = type; d.NumDescriptors = count;
		d.Flags = shaderVis ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		CHECK_HR(pDevice->CreateDescriptorHeap(&d, IID_PPV_ARGS(&outHeap.heap)), "CreateDescriptorHeap");
		outHeap.stride = pDevice->GetDescriptorHandleIncrementSize(type);
		outHeap.capacity = count;
	}

	static D3D12_STATIC_SAMPLER_DESC MakeStaticSampler(u32 reg, D3D12_FILTER filter,
		D3D12_TEXTURE_ADDRESS_MODE address, D3D12_STATIC_BORDER_COLOR border,
		u32 maxAniso, D3D12_COMPARISON_FUNC compare)
	{
		D3D12_STATIC_SAMPLER_DESC s = {};
		s.Filter = filter;
		s.AddressU = address;
		s.AddressV = address;
		s.AddressW = address;
		s.MipLODBias = 0.0f;
		s.MaxAnisotropy = maxAniso;
		s.ComparisonFunc = compare;
		s.BorderColor = border;
		s.MinLOD = 0.0f;
		s.MaxLOD = D3D12_FLOAT32_MAX;
		s.ShaderRegister = reg;
		s.RegisterSpace = 0;
		s.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		return s;
	}

	static void CreateGlobalRootSignature(DX12Device* pDevice)
	{
		const D3D12_STATIC_SAMPLER_DESC samplers[] =
		{
			MakeStaticSampler(0,  D3D12_FILTER_MIN_MAG_MIP_POINT,  D3D12_TEXTURE_ADDRESS_MODE_WRAP,   D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK, 1,  D3D12_COMPARISON_FUNC_NEVER),
			MakeStaticSampler(1,  D3D12_FILTER_MIN_MAG_MIP_POINT,  D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK, 1,  D3D12_COMPARISON_FUNC_NEVER),
			MakeStaticSampler(2,  D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP,   D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK, 1,  D3D12_COMPARISON_FUNC_NEVER),
			MakeStaticSampler(3,  D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK, 1,  D3D12_COMPARISON_FUNC_NEVER),
			MakeStaticSampler(4,  D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_MIRROR, D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK, 1,  D3D12_COMPARISON_FUNC_NEVER),
			MakeStaticSampler(5,  D3D12_FILTER_ANISOTROPIC,        D3D12_TEXTURE_ADDRESS_MODE_WRAP,   D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK, 16, D3D12_COMPARISON_FUNC_NEVER),
			MakeStaticSampler(6,  D3D12_FILTER_ANISOTROPIC,        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK, 16, D3D12_COMPARISON_FUNC_NEVER),
			MakeStaticSampler(7,  D3D12_FILTER_ANISOTROPIC,        D3D12_TEXTURE_ADDRESS_MODE_MIRROR, D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK, 16, D3D12_COMPARISON_FUNC_NEVER),
			MakeStaticSampler(8,  D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_BORDER, D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK, 1,  D3D12_COMPARISON_FUNC_NEVER),
			MakeStaticSampler(9,  D3D12_FILTER_MIN_MAG_MIP_POINT,  D3D12_TEXTURE_ADDRESS_MODE_BORDER, D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE,      1,  D3D12_COMPARISON_FUNC_NEVER),
			MakeStaticSampler(10, D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE, 1, D3D12_COMPARISON_FUNC_LESS_EQUAL),
		};

		D3D12_ROOT_PARAMETER1 rootConstants = {};
		rootConstants.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		rootConstants.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		rootConstants.Constants.ShaderRegister = 0;
		rootConstants.Constants.RegisterSpace = 0;
		rootConstants.Constants.Num32BitValues = 32;

		D3D12_VERSIONED_ROOT_SIGNATURE_DESC versioned = {};
		versioned.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
		versioned.Desc_1_1.NumParameters = 1;
		versioned.Desc_1_1.pParameters = &rootConstants;
		versioned.Desc_1_1.NumStaticSamplers = _countof(samplers);
		versioned.Desc_1_1.pStaticSamplers = samplers;
		versioned.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED;

		ID3DBlob* blob = nullptr;
		ID3DBlob* error = nullptr;
		HRESULT hresult = D3D12SerializeVersionedRootSignature(&versioned, &blob, &error);
		if (FAILED(hresult))
		{
			if (error)
				std::println("RootSignature serialize: {}", (const char*)error->GetBufferPointer());
			CHECK_HR(hresult, "D3D12SerializeVersionedRootSignature");
		}

		CHECK_HR(pDevice->device->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(),
			IID_PPV_ARGS(&pDevice->globalRootSignature)), "CreateRootSignature");

		if (blob)
			blob->Release();
		if (error)
			error->Release();
	}

	GfxDevice::GfxDevice(const GfxDeviceDesc& desc)
	{
		DX12Device* pNative = new DX12Device();

		u32 factoryFlags = 0;
#if defined(_DEBUG)
		if (desc.enableDebug && SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pNative->debug))))
		{
			pNative->debug->EnableDebugLayer();
			factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;

			if (desc.enableGPUValidation)
			{
				ID3D12Debug1* gpuValidation = nullptr;
				if (SUCCEEDED(pNative->debug->QueryInterface(IID_PPV_ARGS(&gpuValidation))))
				{
					gpuValidation->SetEnableGPUBasedValidation(true);
					gpuValidation->Release();
				}
			}
		}
#endif

		HRESULT hresult = CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&pNative->factory));
		CHECK_HR(hresult, "CreateDXGIFactory2");

		hresult = pNative->factory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
			IID_PPV_ARGS(&pNative->adapter));
		CHECK_HR(hresult, "EnumAdapterByGpuPreference");

		ID3D12Device* base = nullptr;
		hresult = D3D12CreateDevice(pNative->adapter, D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&base));
		CHECK_HR(hresult, "D3D12CreateDevice");
		{
			hresult = base->QueryInterface(IID_PPV_ARGS(&pNative->device));
			CHECK_HR(hresult, "D3D12Device to D3DDevice10");
			base->Release();
		}

		const D3D12_COMMAND_LIST_TYPE qtypes[3] =
		{
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			D3D12_COMMAND_LIST_TYPE_COMPUTE,
			D3D12_COMMAND_LIST_TYPE_COPY
		};

		for (u32 queueIdx = 0; queueIdx < 3; queueIdx++)
		{
			D3D12_COMMAND_QUEUE_DESC queueDesc = {};
			queueDesc.Type = qtypes[queueIdx];
			queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
			hresult = pNative->device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&pNative->queues[queueIdx]));
			CHECK_HR(hresult, "CreateCommandQueue");
		}

		D3D12MA::ALLOCATOR_DESC allocDesc = {};
		allocDesc.pDevice = pNative->device;
		allocDesc.pAdapter = pNative->adapter;
		D3D12MA::CreateAllocator(&allocDesc, &pNative->allocator);

		CreateDescHeap(pNative->device, pNative->bindless, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, MaxBindless, true);
		CreateDescHeap(pNative->device, pNative->rtv, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 256, false);
		CreateDescHeap(pNative->device, pNative->dsv, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 64, false);

		CHECK_HR(pNative->device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pNative->frameFence)), "CreateFence");
		pNative->frameFenceEvent = CreateEvent(nullptr, false, false, nullptr);

#if defined(_DEBUG)
		if (SUCCEEDED(pNative->device->QueryInterface(IID_PPV_ARGS(&pNative->infoQueue1))))
		{
			auto callback = [](D3D12_MESSAGE_CATEGORY category, D3D12_MESSAGE_SEVERITY severity,
				D3D12_MESSAGE_ID messageId, LPCSTR desc, void* GfxContext)
				{
					if (severity == D3D12_MESSAGE_SEVERITY_ERROR || severity == D3D12_MESSAGE_SEVERITY_CORRUPTION)
						std::println("[D3D12] {}", desc);
					else if (severity == D3D12_MESSAGE_SEVERITY_WARNING)
						std::println("[D3D12][WARN] {}", desc);
					else
						std::println("[D3D12] {}", desc);
				};

			pNative->infoQueue1->RegisterMessageCallback(callback, D3D12_MESSAGE_CALLBACK_FLAG_NONE, nullptr, &pNative->iqCookie);
		}
#endif
		CreateGlobalRootSignature(pNative);

		pNative->framesInFlight = desc.framesInFlight;
		pNative->maxWorkers = desc.maxWorkers;
		pNative->frames.resize(desc.framesInFlight);
		for (DX12Frame& frame : pNative->frames)
		{
			frame.workers.resize(desc.maxWorkers);
			for (DX12WorkerCmd& worker : frame.workers)
				CHECK_HR(pNative->device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
					IID_PPV_ARGS(&worker.allocator)), "CreateCommandAllocator");
		}

		m_native = pNative;
	}

	GfxDevice::~GfxDevice()
	{
		DX12Device* native = static_cast<DX12Device*>(m_native);
		if (!native)
			return;

		if (native->infoQueue1)
		{
			native->infoQueue1->UnregisterMessageCallback(native->iqCookie);
			native->infoQueue1->Release();
			native->infoQueue1 = nullptr;
		}

		WaitIdle();

		if (native->frameFenceEvent)
			CloseHandle(native->frameFenceEvent);

		if (native->frameFence)
			native->frameFence->Release();

		if (native->bindless.heap)
			native->bindless.heap->Release();
		if (native->rtv.heap)
			native->rtv.heap->Release();
		if (native->dsv.heap)
			native->dsv.heap->Release();

		if (native->globalRootSignature)
			native->globalRootSignature->Release();

		if (native->allocator)
			native->allocator->Release();

		for (ID3D12CommandQueue* queue : native->queues)
			if (queue)
				queue->Release();

		if (native->device)
			native->device->Release();
		if (native->adapter)
			native->adapter->Release();
		if (native->factory)
			native->factory->Release();

#if defined(_DEBUG)
		if (native->infoQueue1)
			native->infoQueue1->Release();
		if (native->debug)
			native->debug->Release();
#endif

		for (DX12Frame& frame : native->frames)
			for (DX12WorkerCmd& worker : frame.workers)
			{
				worker.lists.clear();
				if (worker.allocator)
					worker.allocator->Release();
			}

		delete native;
		m_native = nullptr;
	}

	std::unique_ptr<GfxBuffer> GfxDevice::CreateBuffer(const GfxBufferDesc& desc)
	{
		return std::make_unique<GfxBuffer>(desc, this);
	}

	std::unique_ptr<GfxTexture> GfxDevice::CreateTexture(const GfxTextureDesc& desc)
	{
		return std::make_unique<GfxTexture>(desc, this);
	}

	std::unique_ptr<GfxPipeline> GfxDevice::CreatePipeline(const GfxGraphicsPipelineDesc& desc)
	{
		return std::make_unique<GfxPipeline>(desc, this);
	}

	std::unique_ptr<GfxPipeline> GfxDevice::CreatePipeline(const GfxComputePipelineDesc& desc)
	{
		return std::make_unique<GfxPipeline>(desc, this);
	}

	void GfxDevice::DeferDestroy(std::function<void()> deleter) const
	{
		DX12Device* native = static_cast<DX12Device*>(m_native);

		std::lock_guard<std::mutex> lock(native->deleteMutex);
		native->pendingDeletes.push_back({ native->frameValue + 1, std::move(deleter) });
	}

	void GfxDevice::CollectGarbage() const
	{
		DX12Device* native = static_cast<DX12Device*>(m_native);
		const u64 completed = native->frameFence->GetCompletedValue();

		std::vector<std::function<void()>> ready;
		{
			std::lock_guard<std::mutex> lock(native->deleteMutex);
			std::vector<DX12PendingDelete>& queue = native->pendingDeletes;

			usize write = 0;
			for (usize read = 0; read < queue.size(); ++read)
			{
				if (queue[read].value <= completed)
					ready.push_back(std::move(queue[read].fn));
				else
					queue[write++] = std::move(queue[read]);
			}
			queue.resize(write);
		}

		for (std::function<void()>& fn : ready)
			fn();
	}

	void GfxDevice::BeginFrame()
	{
		DX12Device* native = static_cast<DX12Device*>(m_native);
		DX12Frame& frame = native->frames[native->frameIndex];

		if (native->frameFence->GetCompletedValue() < frame.fenceValue)
		{
			native->frameFence->SetEventOnCompletion(frame.fenceValue, native->frameFenceEvent);
			WaitForSingleObjectEx(native->frameFenceEvent, INFINITE, false);
		}

		for (DX12WorkerCmd& worker : frame.workers)
		{
			worker.allocator->Reset();
			worker.nextList = 0;
		}

		CollectGarbage();
	}

	void GfxDevice::EndFrame()
	{
		DX12Device* native = static_cast<DX12Device*>(m_native);

		native->frameValue++;
		native->queues[u32(GfxQueueType::Graphics)]->Signal(native->frameFence, native->frameValue);
		native->frames[native->frameIndex].fenceValue = native->frameValue;
		native->frameIndex = (native->frameIndex + 1) % native->framesInFlight;
	}

	GfxCommandList* GfxDevice::AcquireCmd(u32 worker)
	{
		DX12Device* native = static_cast<DX12Device*>(m_native);
		DX12WorkerCmd& w = native->frames[native->frameIndex].workers[worker];

		if (w.nextList >= w.lists.size())
			w.lists.push_back(std::make_unique<GfxCommandList>(this, GfxQueueType::Graphics, w.allocator));

		GfxCommandList* cmd = w.lists[w.nextList++].get();
		cmd->Reset(w.allocator);
		return cmd;
	}

	void GfxDevice::Submit(GfxQueueType type, std::span<GfxCommandList* const> lists)
	{
		DX12Device* native = static_cast<DX12Device*>(m_native);

		std::vector<ID3D12CommandList*> raw(lists.size());
		for (usize i = 0; i < lists.size(); ++i)
			raw[i] = static_cast<ID3D12GraphicsCommandList6*>(lists[i]->GetNative());

		native->queues[u32(type)]->ExecuteCommandLists((u32)raw.size(), raw.data());
	}

	void GfxDevice::WaitIdle() const
	{
		DX12Device* native = static_cast<DX12Device*>(m_native);

		native->frameValue++;
		native->queues[u32(GfxQueueType::Graphics)]->Signal(native->frameFence, native->frameValue);

		if (native->frameFence->GetCompletedValue() < native->frameValue)
		{
			native->frameFence->SetEventOnCompletion(native->frameValue, native->frameFenceEvent);
			WaitForSingleObjectEx(native->frameFenceEvent, INFINITE, false);
		}

		CollectGarbage();
	}
}