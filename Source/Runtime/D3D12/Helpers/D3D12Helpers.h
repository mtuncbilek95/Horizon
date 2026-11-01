#pragma once

#include <Runtime/RHI/Texture/TextureAspect.h>
#include <Runtime/RHI/Texture/TextureFormat.h>
#include <Runtime/RHI/Descriptor/DescriptorType.h>
#include <Runtime/RHI/Pipeline/ColorTarget.h>
#include <Runtime/RHI/Pipeline/DepthStencilState.h>
#include <Runtime/RHI/Pipeline/PrimitiveTopology.h>
#include <Runtime/RHI/Pipeline/RasterizerState.h>
#include <Runtime/RHI/Buffer/IndexType.h>
#include <Runtime/RHI/Command/LoadOp.h>
#include <Runtime/RHI/Command/ResourceState.h>
#include <Runtime/RHI/Command/StoreOp.h>
#include <Runtime/RHI/Queue/QueueType.h>
#include <Runtime/RHI/Sampler/SamplerDesc.h>
#include <Runtime/Containers/Vector/ReadOnlyList.h>

#include <dxgi1_6.h>
#include <d3d12.h>

namespace Horizon::RHI
{
	class TextureView;

	struct Helpers
	{
		static DXGI_FORMAT ToFormat(TextureFormat format);
		static DXGI_FORMAT ToTypelessFormat(TextureFormat format);
		static DXGI_FORMAT ToShaderViewFormat(TextureFormat format, TextureAspect aspect);
		static b8 IsDepthFormat(TextureFormat format);

		static D3D12_SHADER_RESOURCE_VIEW_DESC ToShaderResourceViewDesc(const TextureView& view);
		static D3D12_UNORDERED_ACCESS_VIEW_DESC ToUnorderedAccessViewDesc(const TextureView& view);
		static D3D12_RENDER_TARGET_VIEW_DESC ToRenderTargetViewDesc(const TextureView& view);
		static D3D12_DEPTH_STENCIL_VIEW_DESC ToDepthStencilViewDesc(const TextureView& view);

		static D3D12_DESCRIPTOR_RANGE_TYPE ToDescriptorRangeType(DescriptorType type);

		static D3D12_BLEND ToBlend(BlendFactor factor);
		static D3D12_BLEND_OP ToBlendOp(BlendOp op);
		static D3D12_COMPARISON_FUNC ToCompareFunc(CompareOp op);
		static D3D12_STENCIL_OP ToStencilOp(StencilOp op);
		static D3D12_FILL_MODE ToFillMode(FillMode mode);
		static D3D12_CULL_MODE ToCullMode(CullMode mode);
		static D3D12_PRIMITIVE_TOPOLOGY_TYPE ToPrimitiveTopologyType(PrimitiveTopology topology);
		static D3D12_PRIMITIVE_TOPOLOGY ToPrimitiveTopology(PrimitiveTopology topology);

		static D3D12_RASTERIZER_DESC ToRasterizerDesc(const RasterizerState& state);
		static D3D12_DEPTH_STENCIL_DESC ToDepthStencilDesc(const DepthStencilState& state);
		static D3D12_BLEND_DESC ToBlendDesc(ReadOnlyList<ColorTarget> targets, b8 alphaToCoverage);

		static D3D12_COMMAND_LIST_TYPE ToCommandListType(QueueType type);
		static DXGI_FORMAT ToIndexFormat(IndexType type);

		static D3D12_BARRIER_SYNC ToBarrierSync(ResourceState state);
		static D3D12_BARRIER_ACCESS ToBarrierAccess(ResourceState state);
		static D3D12_BARRIER_LAYOUT ToBarrierLayout(ResourceState state);

		static D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE ToBeginningAccess(LoadOp op);
		static D3D12_RENDER_PASS_ENDING_ACCESS_TYPE ToEndingAccess(StoreOp op);

		static D3D12_TEXTURE_ADDRESS_MODE ToAddressMode(SamplerAddressMode mode);
		static D3D12_SAMPLER_DESC ToSamplerDesc(const SamplerDesc& desc);
	};
}