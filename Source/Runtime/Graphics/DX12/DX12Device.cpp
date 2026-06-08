#include "DX12Context.h"

namespace Horizon
{
	void GfxDevice::InitializeDevice(const GfxDeviceDesc& desc)
	{
		Context& context = DX12Context();
		context.bEnableDebug = desc.enableDebug;
		context.bGPUValidation = desc.enableGPUValidation;
		context.pCmdWorkerCount = desc.cmdWorkerCount;

		u32 factoryFlags = 0;
		if (desc.enableDebug)
		{
			HRESULT bResult = D3D12GetDebugInterface(IID_PPV_ARGS(&context.pDebug));
			CHECK_HR(bResult, "ID3D12Debug - D3D12GetDebugInterface");

			context.pDebug->EnableDebugLayer();
			factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;

			ID3D12Debug1* gpuValidation = nullptr;
			bResult = context.pDebug->QueryInterface(IID_PPV_ARGS(&gpuValidation));
			CHECK_HR(bResult, "ID3D12Debug1 - QueryInterface");

			if (desc.enableGPUValidation)
				gpuValidation->SetEnableGPUBasedValidation(true);

			gpuValidation->Release();
		}

		HRESULT bResult = CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&context.pFactory));
		CHECK_HR(bResult, "IDXGIFactory7 - CreateDXGIFactory2");

		bResult = context.pFactory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
			IID_PPV_ARGS(&context.pAdapter));
		CHECK_HR(bResult, "IDXGIAdapter4 - EnumAdapterByGpuPreference");

		ID3D12Device* pBase = nullptr;
		bResult = D3D12CreateDevice(context.pAdapter, D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&pBase));
		CHECK_HR(bResult, "ID3D12Device - D3D12CreateDevice");
		{
			bResult = pBase->QueryInterface(IID_PPV_ARGS(&context.pDevice));
			CHECK_HR(bResult, "ID3D12Device10 - QueryInterface");
			pBase->Release();
		}

		const D3D12_COMMAND_LIST_TYPE qTypes[3] =
		{
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			D3D12_COMMAND_LIST_TYPE_COMPUTE,
			D3D12_COMMAND_LIST_TYPE_COPY
		};

		for (usize i = 0; i < context.queuePool.size(); i++)
		{
			D3D12_COMMAND_QUEUE_DESC queueDesc = {};
			queueDesc.Type = qTypes[i];
			queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
			bResult = context.pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&context.queuePool[i]));
			CHECK_HR(bResult, "ID3D12CommandQueue - CreateCommandQueue");
		}

		D3D12MA::ALLOCATOR_DESC allocDesc = {};
		allocDesc.pDevice = context.pDevice;
		allocDesc.pAdapter = context.pAdapter;
		D3D12MA::CreateAllocator(&allocDesc, &context.pAllocator);

		Helpers::CreateDescriptorHeap(context.resourceHeap, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, BindlessCapacity, true);
		Helpers::CreateDescriptorHeap(context.renderTargetHeap, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1024, false);
		Helpers::CreateDescriptorHeap(context.depthStencilHeap, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 256, false);

#if defined(HORIZON_DEBUG)
		Helpers::CreateTerminalLog();
#endif
	}

	void GfxDevice::ShutdownDevice()
	{
		Context& context = DX12Context();

		if (context.pInfoQueue)
		{
			context.pInfoQueue->UnregisterMessageCallback(context.pInfoId);
			context.pInfoQueue->Release();
			context.pInfoQueue = nullptr;
		}
	}

	void GfxDevice::WaitIdle()
	{}
}