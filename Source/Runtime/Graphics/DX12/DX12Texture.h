#pragma once

#include <dxgi1_6.h>
#include <d3d12.h>
#include <comdef.h>
#include <D3D12MemAlloc.h>

namespace Horizon
{
	struct DX12Texture
	{
		ID3D12Resource* resource = nullptr;
		D3D12MA::Allocation* memory = nullptr;
		D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON;
		DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
		b8 isBackbuffer = false;
	};
}
