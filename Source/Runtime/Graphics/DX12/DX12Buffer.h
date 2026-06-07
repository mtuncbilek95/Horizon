#pragma once

#include <dxgi1_6.h>
#include <d3d12.h>
#include <comdef.h>
#include <D3D12MemAlloc.h>

namespace Horizon
{
	struct DX12Buffer
	{
		ID3D12Resource* resource = nullptr;
		D3D12MA::Allocation* memory = nullptr;
	};
}