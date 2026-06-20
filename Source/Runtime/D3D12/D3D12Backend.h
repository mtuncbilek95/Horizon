#pragma once

#include <Runtime/RHI/GfxTypes.h>

#include <dxgi1_6.h>
#include <d3d12.h>
#include <comdef.h>
#include <D3D12MemAlloc.h>

namespace Horizon
{
#define CHECK_HR(hr, what) \
	if(FAILED(hr)) { Terminal::Error("D3D12", "{}: {}", what, _com_error(hr).ErrorMessage()); exit(-1); }

#define CHECK_REASON(hr, what) \
	if(FAILED(hr)) { Terminal::Error("D3D12", "{}: {}", what, _com_error(hr).ErrorMessage()); }

	namespace Helpers
	{
		DXGI_FORMAT ToDXGIFormat(GfxTextureFormat format);
		DXGI_FORMAT ToTypelessFormat(DXGI_FORMAT format);
		DXGI_FORMAT ToDepthSRVFormat(DXGI_FORMAT format);
		b8 IsDepthFormat(GfxTextureFormat format);

		D3D12_BLEND ToBlend(GfxBlendFactor factor);
		D3D12_BLEND_OP ToBlendOp(GfxBlendOp op);
		D3D12_COMPARISON_FUNC ToCompare(GfxCompareOp op);
		D3D12_PRIMITIVE_TOPOLOGY_TYPE ToTopologyType(GfxPrimitiveTopology topology);
		D3D_PRIMITIVE_TOPOLOGY ToTopology(GfxPrimitiveTopology topology);
		D3D12_RESOURCE_STATES ToResourceState(GfxResourceState state);
		D3D12_COMMAND_LIST_TYPE ToListType(GfxQueueType type);
	}
}