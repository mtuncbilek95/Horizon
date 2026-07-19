#pragma once

#include <Runtime/Log/Terminal.h>
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
		DXGI_FORMAT ToIndexFormat(GfxIndexType type);
		b8 IsDepthFormat(GfxTextureFormat format);

		D3D12_BLEND ToBlend(GfxBlendFactor factor);
		D3D12_BLEND_OP ToBlendOp(GfxBlendOp op);
		D3D12_COMPARISON_FUNC ToCompare(GfxCompareOp op);
		D3D12_STENCIL_OP ToStencilOp(GfxStencilOp op);
		u8 ToColorWriteMask(GfxColorWrite mask);

		D3D12_FILL_MODE ToFillMode(GfxFillMode mode);
		D3D12_CULL_MODE ToCullMode(GfxCullMode mode);
		b8 ToFrontCounterClockwise(GfxFrontFace face);

		D3D12_PRIMITIVE_TOPOLOGY_TYPE ToTopologyType(GfxPrimitiveTopology topology);
		D3D_PRIMITIVE_TOPOLOGY ToTopology(GfxPrimitiveTopology topology);

		D3D12_RESOURCE_STATES ToResourceState(GfxResourceState state);
		D3D12_HEAP_TYPE ToHeapType(GfxMemoryType type);
		D3D12_DESCRIPTOR_HEAP_TYPE ToDescriptorHeapType(GfxDescriptorHeapType type);
		D3D12_RESOURCE_DIMENSION ToResourceDimension(GfxTextureType type);
		D3D12_RESOURCE_FLAGS ToResourceFlags(GfxTextureUsage usage);
		D3D12_RESOURCE_FLAGS ToResourceFlags(GfxBufferUsage usage);

		D3D12_FILTER ToFilter(GfxFilter minFilter, GfxFilter magFilter, GfxMipFilter mipFilter, b8 anisotropy, b8 comparison);
		D3D12_TEXTURE_ADDRESS_MODE ToAddressMode(GfxAddressMode mode);
		D3D12_STATIC_BORDER_COLOR ToStaticBorderColor(GfxBorderColor color);

		D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE ToBeginAccess(GfxLoadOp op);
		D3D12_RENDER_PASS_ENDING_ACCESS_TYPE ToEndAccess(GfxStoreOp op);

		D3D12_COMMAND_LIST_TYPE ToListType(GfxQueueType type);
		u32 ToSampleCount(GfxSampleCount count);
	}
}