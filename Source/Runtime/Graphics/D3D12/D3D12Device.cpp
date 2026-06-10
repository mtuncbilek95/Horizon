#include "D3D12Backend.h"

#include <Runtime/Graphics/GfxBackend.h>

namespace Horizon
{
	GfxDevice* Gfx::CreateGfxDevice(const GfxDeviceDesc& desc)
	{
		GfxDevice* pContext = new GfxDevice();

		u32 factoryFlags = 0;
		if (desc.enableDebug)
		{
			HRESULT bResult = D3D12GetDebugInterface(IID_PPV_ARGS(&pContext->pDebug));
			CHECK_HR(bResult, "ID3D12Debug - D3D12GetDebugInterface");

			pContext->pDebug->EnableDebugLayer();
			factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;

			ID3D12Debug1* gpuValidation = nullptr;
			bResult = pContext->pDebug->QueryInterface(IID_PPV_ARGS(&gpuValidation));
			CHECK_HR(bResult, "ID3D12Debug1 - QueryInterface");

			if (desc.enableGPUValidation)
				gpuValidation->SetEnableGPUBasedValidation(true);

			gpuValidation->Release();
		}

		HRESULT bResult = CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&pContext->pFactory));
		CHECK_HR(bResult, "IDXGIFactory7 - CreateDXGIFactory2");

		bResult = pContext->pFactory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
			IID_PPV_ARGS(&pContext->pAdapter));
		CHECK_HR(bResult, "IDXGIAdapter4 - EnumAdapterByGpuPreference");

		ID3D12Device* pBase = nullptr;
		bResult = D3D12CreateDevice(pContext->pAdapter, D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&pBase));
		CHECK_HR(bResult, "ID3D12Device - D3D12CreateDevice");
		{
			bResult = pBase->QueryInterface(IID_PPV_ARGS(&pContext->pDevice));
			CHECK_HR(bResult, "ID3D12Device10 - QueryInterface");
			pBase->Release();
		}

		D3D12MA::ALLOCATOR_DESC allocDesc = {};
		allocDesc.pDevice = pContext->pDevice;
		allocDesc.pAdapter = pContext->pAdapter;
		D3D12MA::CreateAllocator(&allocDesc, &pContext->pAllocator);

#if defined(HORIZON_DEBUG)
		Helpers::CreateTerminalLog(pContext);
#endif

		return pContext;
	}

	void Gfx::ShutdownGfxDevice(GfxDevice* devHandl)
	{
		if (devHandl->pInfoQueue)
		{
			devHandl->pInfoQueue->UnregisterMessageCallback(devHandl->pInfoId);
			devHandl->pInfoQueue->Release();
			devHandl->pInfoQueue = nullptr;
		}
		
		if (devHandl->pAllocator)
			devHandl->pAllocator->Release();
			
		if (devHandl->pDevice)
			devHandl->pDevice->Release();

		if (devHandl->pAdapter)
			devHandl->pAdapter->Release();
			
		if (devHandl->pFactory)
			devHandl->pFactory->Release();
	}
}