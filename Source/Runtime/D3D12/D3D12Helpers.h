#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/Log/Terminal.h>

#include <Runtime/RHI/Buffer/GfxBufferUsage.h>
#include <Runtime/RHI/Buffer/GfxIndexType.h>
#include <Runtime/RHI/Command/GfxLoadOp.h>
#include <Runtime/RHI/Command/GfxResourceState.h>
#include <Runtime/RHI/Command/GfxStoreOp.h>
#include <Runtime/RHI/Common/GfxSampleCount.h>
#include <Runtime/RHI/Descriptor/GfxDescriptorHeapType.h>
#include <Runtime/RHI/Memory/GfxMemoryType.h>
#include <Runtime/RHI/Pipeline/GfxBlendFactor.h>
#include <Runtime/RHI/Pipeline/GfxBlendOp.h>
#include <Runtime/RHI/Pipeline/GfxColorWrite.h>
#include <Runtime/RHI/Pipeline/GfxCompareOp.h>
#include <Runtime/RHI/Pipeline/GfxCullMode.h>
#include <Runtime/RHI/Pipeline/GfxFillMode.h>
#include <Runtime/RHI/Pipeline/GfxFrontFace.h>
#include <Runtime/RHI/Pipeline/GfxPrimitiveTopology.h>
#include <Runtime/RHI/Pipeline/GfxStencilOp.h>
#include <Runtime/RHI/Queue/GfxQueueType.h>
#include <Runtime/RHI/Swapchain/GfxPresentMode.h>
#include <Runtime/RHI/Texture/GfxTextureFormat.h>
#include <Runtime/RHI/Texture/GfxTextureType.h>
#include <Runtime/RHI/Texture/GfxTextureUsage.h>

#include <dxgi1_6.h>
#include <d3d12.h>
#include <comdef.h>
#include <D3D12MemAlloc.h>

namespace Horizon::RHI
{
#define CHECK_HR(hr, what) \
	if(FAILED(hr)) { Terminal::Error("D3D12", "{}: {}", what, _com_error(hr).ErrorMessage()); exit(-1); }

#define CHECK_REASON(hr, what) \
	if(FAILED(hr)) { Terminal::Error("D3D12", "{}: {}", what, _com_error(hr).ErrorMessage()); }

	struct H_EXPORT Helpers
	{
		static D3D12_DESCRIPTOR_HEAP_TYPE ToDescriptorHeapType(GfxDescriptorHeapType type);
		static D3D12_COMMAND_LIST_TYPE ToCommandListType(GfxQueueType type);

		static DXGI_FORMAT ToFormat(GfxTextureFormat format);
		static DXGI_FORMAT ToIndexFormat(GfxIndexType type);

		static D3D12_HEAP_TYPE ToHeapType(GfxMemoryType type);
		static D3D12_RESOURCE_STATES ToResourceState(GfxResourceState state);
		static D3D12_RESOURCE_DIMENSION ToResourceDimension(GfxTextureType type);
		static D3D12_RESOURCE_FLAGS ToResourceFlags(GfxTextureUsage usage);
		static D3D12_RESOURCE_FLAGS ToResourceFlags(GfxBufferUsage usage);

		static D3D12_FILL_MODE ToFillMode(GfxFillMode mode);
		static D3D12_CULL_MODE ToCullMode(GfxCullMode mode);
		static BOOL ToFrontCounterClockwise(GfxFrontFace face);

		static D3D12_COMPARISON_FUNC ToCompareFunc(GfxCompareOp op);
		static D3D12_STENCIL_OP ToStencilOp(GfxStencilOp op);

		static D3D12_BLEND ToBlend(GfxBlendFactor factor);
		static D3D12_BLEND_OP ToBlendOp(GfxBlendOp op);
		static u8 ToColorWriteMask(GfxColorWrite mask);

		static D3D12_PRIMITIVE_TOPOLOGY_TYPE ToTopologyType(GfxPrimitiveTopology topology);
		static D3D_PRIMITIVE_TOPOLOGY ToTopology(GfxPrimitiveTopology topology);
		static u32 ToSampleCount(GfxSampleCount count);

		static D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE ToBeginAccess(GfxLoadOp op);
		static D3D12_RENDER_PASS_ENDING_ACCESS_TYPE ToEndAccess(GfxStoreOp op);

		static u32 ToSyncInterval(GfxPresentMode mode);
		static u32 ToPresentFlags(GfxPresentMode mode);

		static DXGI_FORMAT ToTypelessFormat(DXGI_FORMAT format);
		static DXGI_FORMAT ToDepthSRVFormat(DXGI_FORMAT format);
		static DXGI_FORMAT ToSwapchainFormat(DXGI_FORMAT format);
		static b8 IsDepthFormat(DXGI_FORMAT format);

		static void SetObjectName(ID3D12Object* pObject, const c8* pName);
	};
}