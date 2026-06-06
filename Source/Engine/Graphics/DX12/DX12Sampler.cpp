#include "DX12Backend.h"
#include <Engine/Graphics/RHI/GfxSampler.h>

namespace Horizon
{
	using namespace DX12;

	static D3D12_TEXTURE_ADDRESS_MODE ToAddress(GfxAddressMode mode)
	{
		switch (mode)
		{
		case GfxAddressMode::Repeat:
			return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		case GfxAddressMode::MirrorRepeat:
			return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
		case GfxAddressMode::ClampEdge:
			return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		case GfxAddressMode::ClampBorder:
			return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		default:
			return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		}
	}

	static D3D12_FILTER ToFilter(const GfxSamplerDesc& desc)
	{
		if (desc.maxAnisotropy > 1)
			return desc.compareEnable ? D3D12_FILTER_COMPARISON_ANISOTROPIC : D3D12_FILTER_ANISOTROPIC;

		D3D12_FILTER_REDUCTION_TYPE reduction = desc.compareEnable
			? D3D12_FILTER_REDUCTION_TYPE_COMPARISON : D3D12_FILTER_REDUCTION_TYPE_STANDARD;
		auto toFilterType = [](GfxFilter filter) { return filter == GfxFilter::Linear ? D3D12_FILTER_TYPE_LINEAR : D3D12_FILTER_TYPE_POINT; };
		return D3D12_ENCODE_BASIC_FILTER(toFilterType(desc.minFilter), toFilterType(desc.magFilter), toFilterType(desc.mipFilter), reduction);
	}

	static void ToBorder(GfxBorderColor border, f32 out[4])
	{
		switch (border)
		{
		case GfxBorderColor::OpaqueWhite: out[0] = out[1] = out[2] = out[3] = 1.0f; break;
		case GfxBorderColor::OpaqueBlack: out[0] = out[1] = out[2] = 0.0f; out[3] = 1.0f; break;
		default: out[0] = out[1] = out[2] = out[3] = 0.0f; break;
		}
	}

	GfxSamplerHandle GfxDevice::CreateSampler(const GfxSamplerDesc& desc)
	{
		Context& context = GfxContext();

		u32 slot;
		if (!context.samplerFreeList.empty())
		{
			slot = context.samplerFreeList.back();
			context.samplerFreeList.pop_back();
		}
		else
			slot = context.nextSampler++;

		D3D12_SAMPLER_DESC samplerDesc = {};
		samplerDesc.Filter = ToFilter(desc);
		samplerDesc.AddressU = ToAddress(desc.addressU);
		samplerDesc.AddressV = ToAddress(desc.addressV);
		samplerDesc.AddressW = ToAddress(desc.addressW);
		samplerDesc.MaxAnisotropy = desc.maxAnisotropy;
		samplerDesc.ComparisonFunc = desc.compareEnable ? D3D12_COMPARISON_FUNC(u32(desc.compareOp) + 1) : D3D12_COMPARISON_FUNC_NEVER;
		samplerDesc.MinLOD = desc.minLod;
		samplerDesc.MaxLOD = desc.maxLod;
		ToBorder(desc.border, samplerDesc.BorderColor);

		auto cpuHandle = context.samplerHeap->GetCPUDescriptorHandleForHeapStart();
		cpuHandle.ptr += SIZE_T(slot) * context.samplerStride;
		context.device->CreateSampler(&samplerDesc, cpuHandle);

		return GfxSamplerHandle::make(slot, 0);
	}

	void GfxDevice::DestroySampler(GfxSamplerHandle handle)
	{
		GfxContext().samplerFreeList.push_back(handle.index());
	}
}
