#pragma once

#include <Runtime/RHI/Sampler/Sampler.h>

#include <dxgi1_6.h>
#include <d3d12.h>

namespace Horizon::RHI
{
	class D3D12Sampler final : public Sampler
	{
		friend class D3D12Device;
	public:
		const D3D12_SAMPLER_DESC& NativeDesc() const { return m_nativeDesc; }

	private:
		D3D12_SAMPLER_DESC m_nativeDesc = {};
	};
}
