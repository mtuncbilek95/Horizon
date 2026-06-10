#pragma once

#include <Runtime/Log/Log.h>

#include <dxgi1_6.h>
#include <d3d12.h>
#include <comdef.h>
#include <D3D12MemAlloc.h>

namespace Horizon
{
#define CHECK_HR(hr, what) \
	if(FAILED(hr)) { MainLog::Error("{}: {}", what, _com_error(hr).ErrorMessage()); exit(-1); }

#define CHECK_REASON(hr, what) \
	if(FAILED(hr)) { MainLog::Error("{}: {}", what, _com_error(hr).ErrorMessage()); }

	struct GfxBuffer
	{};

	struct GfxTexture
	{};

	struct GfxPipeline
	{};

	struct GfxCmdAllocator
	{};

	struct GfxCmdList
	{};



	struct GfxDevice
	{
#if defined(HORIZON_DEBUG)
		ID3D12Debug* pDebug = nullptr;
		ID3D12InfoQueue1* pInfoQueue = nullptr;
		DWORD pInfoId = 0;
#endif

		IDXGIFactory7* pFactory = nullptr;
		IDXGIAdapter4* pAdapter = nullptr;
		ID3D12Device10* pDevice = nullptr;
		D3D12MA::Allocator* pAllocator = nullptr;
	};

	struct GfxSwapchain
	{
		IDXGISwapChain4* pSwapchain = nullptr;
	};

	namespace Helpers
	{
		void CreateTerminalLog(GfxDevice* pContext);
	}
}