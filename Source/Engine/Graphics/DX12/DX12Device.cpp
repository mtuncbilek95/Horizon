#include "DX12Backend.h"

#include <Engine/Log/Log.h>

#define CHECK_HR(hr, what) \
	if(FAILED(hr)) { ConsoleLog().Error("{}: {}", what, _com_error(hr).ErrorMessage()); exit(-1); }

namespace Horizon::DX12
{
	static Context gContext;
	Context& GfxContext() { return gContext; }

	static void CreateHeap(ID3D12DescriptorHeap** outHeap, D3D12_DESCRIPTOR_HEAP_TYPE type, u32 count, b8 shaderVisible)
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = type;
		desc.NumDescriptors = count;
		desc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		CHECK_HR(gContext.device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(outHeap)), "CreateDescriptorHeap");
	}

	u32 BindlessAlloc()
	{
		Context& context = GfxContext();
		if (!context.bindlessFreeList.empty()) { u32 slot = context.bindlessFreeList.back(); context.bindlessFreeList.pop_back(); return slot; }
		return context.nextBindless++;
	}

	void BindlessFree(u32 slot) { GfxContext().bindlessFreeList.push_back(slot); }

	D3D12_CPU_DESCRIPTOR_HANDLE BindlessCpu(u32 slot)
	{
		Context& context = GfxContext();
		auto handle = context.bindlessHeap->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += SIZE_T(slot) * context.bindlessStride;
		return handle;
	}

	D3D12_GPU_DESCRIPTOR_HANDLE BindlessGpu(u32 slot)
	{
		Context& context = GfxContext();
		auto handle = context.bindlessHeap->GetGPUDescriptorHandleForHeapStart();
		handle.ptr += UINT64(slot) * context.bindlessStride;
		return handle;
	}
}

namespace Horizon
{
	void GfxDevice::Initialize(const GfxDeviceDesc& desc)
	{
		using namespace DX12;
		Context& context = GfxContext();
		context.framesInFlight = desc.framesInFlight;
		context.maxWorkers = desc.maxWorkers;
		context.frames.resize(desc.framesInFlight);

		u32 factoryFlags = 0;
#if defined(_DEBUG)
		if (desc.enableDebug && SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&context.debug))))
		{
			context.debug->EnableDebugLayer();
			factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;

			ID3D12Debug1* gpuValidation = nullptr;
			if (SUCCEEDED(context.debug->QueryInterface(IID_PPV_ARGS(&gpuValidation))))
			{
				//gpuValidation->SetEnableGPUBasedValidation(true);
				gpuValidation->Release();
			}
		}
#endif

		HRESULT hresult = CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&context.factory));
		CHECK_HR(hresult, "CreateDXGIFactory2");

		hresult = context.factory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
			IID_PPV_ARGS(&context.adapter));
		CHECK_HR(hresult, "EnumAdapterByGpuPreference");

		ID3D12Device* base = nullptr;
		hresult = D3D12CreateDevice(context.adapter, D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&base));
		CHECK_HR(hresult, "D3D12CreateDevice");
		{
			hresult = base->QueryInterface(IID_PPV_ARGS(&context.device));
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
			hresult = context.device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&context.queues[queueIdx]));
			CHECK_HR(hresult, "CreateCommandQueue");
		}

		D3D12MA::ALLOCATOR_DESC allocDesc = {};
		allocDesc.pDevice = context.device;
		allocDesc.pAdapter = context.adapter;
		D3D12MA::CreateAllocator(&allocDesc, &context.allocator);

		CreateHeap(&context.bindlessHeap, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, BindlessCapacity, true);
		CreateHeap(&context.targetViewHeap, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 256, false);
		CreateHeap(&context.depthViewHeap, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 64, false);
		CreateHeap(&context.samplerHeap, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 2048, true);

		context.samplerStride = context.device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

		context.bindlessStride = context.device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		context.targetViewStride = context.device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		context.depthViewStride = context.device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

		hresult = context.device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&context.frameFence));
		CHECK_HR(hresult, "CreateFence");
		context.frameFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

		CreateGlobalRootSignature();

		for (u32 frameIdx = 0; frameIdx < context.framesInFlight; frameIdx++)
		{
			for (u32 workerIdx = 0; workerIdx < context.maxWorkers; workerIdx++)
				context.device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&context.frames[frameIdx].workers[workerIdx].allocator));
		}

#if defined(_DEBUG)
		if (SUCCEEDED(context.device->QueryInterface(IID_PPV_ARGS(&context.infoQueue1))))
		{
			auto callback = [](D3D12_MESSAGE_CATEGORY category, D3D12_MESSAGE_SEVERITY severity,
				D3D12_MESSAGE_ID messageId, LPCSTR desc, void* GfxContext)
				{
					if (severity == D3D12_MESSAGE_SEVERITY_ERROR || severity == D3D12_MESSAGE_SEVERITY_CORRUPTION)
						ConsoleLog().Error("[D3D12] {}", desc);
					else if (severity == D3D12_MESSAGE_SEVERITY_WARNING)
						ConsoleLog().Error("[D3D12][WARN] {}", desc);
					else
						ConsoleLog().Info("[D3D12] {}", desc);
				};

			context.infoQueue1->RegisterMessageCallback(callback, D3D12_MESSAGE_CALLBACK_FLAG_NONE, nullptr, &context.iqCookie);
		}

		SetDebugName(context.queues[0], "GraphicsQueue");
		SetDebugName(context.queues[1], "ComputeQueue");
		SetDebugName(context.queues[2], "CopyQueue");
		
		SetDebugName(context.bindlessHeap, "BindlessHeap");
		SetDebugName(context.targetViewHeap, "RTVHeap");
		SetDebugName(context.depthViewHeap, "DSVHeap");
		SetDebugName(context.samplerHeap, "SamplerHeap");
		
		SetDebugName(context.frameFence, "FrameFence");
		SetDebugName(context.rootSignature, "GlobalRootSig");
#endif
	}

	void GfxDevice::Shutdown()
	{
		using namespace DX12;
		Context& context = GfxContext();

		if (context.infoQueue1)
		{
			context.infoQueue1->UnregisterMessageCallback(context.iqCookie);
			context.infoQueue1->Release();
			context.infoQueue1 = nullptr;
		}

		WaitIdle();

		for (u32 frameIdx = 0; frameIdx < context.framesInFlight; frameIdx++)
		{
			for (u32 workerIdx = 0; workerIdx < context.maxWorkers; workerIdx++)
			{
				WorkerCmd& worker = context.frames[frameIdx].workers[workerIdx];
				for (u32 listIdx = 0; listIdx < worker.listCount; listIdx++)
				{
					if (worker.lists[listIdx].list)
						worker.lists[listIdx].list->Release();
				}

				if (worker.allocator)
					worker.allocator->Release();
			}
		}

		for (DX12Pipeline& pipeline : context.pipelines)
			if (pipeline.pso)
				pipeline.pso->Release();

		for (auto* signature : context.cmdSigs)
			if (signature)
				signature->Release();

		for (DX12Buffer& buffer : context.buffers)
		{
			if (buffer.resource)
				buffer.resource->Release();
			if (buffer.memory)
				buffer.memory->Release();
		}

		for (DX12Texture& texture : context.textures)
		{
			if (texture.resource && !texture.isBackbuffer)
				texture.resource->Release();
			if (texture.memory)
				texture.memory->Release();
		}

		if (context.rootSignature)
			context.rootSignature->Release();

		if (context.frameFenceEvent)
			CloseHandle(context.frameFenceEvent);

		if (context.frameFence)
			context.frameFence->Release();

		if (context.bindlessHeap)
			context.bindlessHeap->Release();
		if (context.targetViewHeap)
			context.targetViewHeap->Release();
		if (context.depthViewHeap)
			context.depthViewHeap->Release();
		if (context.samplerHeap)
			context.samplerHeap->Release();

		if (context.allocator)
			context.allocator->Release();

		if (context.swapchain)
			context.swapchain->Release();

		for (auto* queue : context.queues)
		{
			if (queue)
				queue->Release();
		}

		if (context.device)
			context.device->Release();
		if (context.adapter)
			context.adapter->Release();
		if (context.factory)
			context.factory->Release();

#if defined(_DEBUG)
		if (context.debug)
			context.debug->Release();
#endif
	}

	void GfxDevice::WaitIdle()
	{
		using namespace DX12;
		Context& context = GfxContext();
		context.frameFenceValue++;
		context.queues[u32(GfxQueueType::Graphics)]->Signal(context.frameFence, context.frameFenceValue);

		if (context.frameFence->GetCompletedValue() < context.frameFenceValue)
		{
			context.frameFence->SetEventOnCompletion(context.frameFenceValue, context.frameFenceEvent);
			WaitForSingleObjectEx(context.frameFenceEvent, INFINITE, false);
		}
	}

	void GfxDevice::BeginFrame()
	{
		using namespace DX12;
		Context& context = GfxContext();
		Frame& frame = context.frames[context.frameIndex];

		if (context.frameFence->GetCompletedValue() < frame.fenceValue)
		{
			context.frameFence->SetEventOnCompletion(frame.fenceValue, context.frameFenceEvent);
			WaitForSingleObjectEx(context.frameFenceEvent, INFINITE, false);
		}

		for (u32 workerIdx = 0; workerIdx < context.maxWorkers; workerIdx++)
		{
			frame.workers[workerIdx].allocator->Reset();
			frame.workers[workerIdx].nextList = 0;
		}
	}

	void GfxDevice::EndFrame()
	{
		using namespace DX12;
		Context& context = GfxContext();

		context.frameFenceValue++;
		context.queues[u32(GfxQueueType::Graphics)]->Signal(context.frameFence, context.frameFenceValue);
		context.frames[context.frameIndex].fenceValue = context.frameFenceValue;
		context.frameIndex = (context.frameIndex + 1) % context.framesInFlight;
	}
}