#pragma once

#include <Runtime/RHI/Sampler/GfxSampler.h>
#include <Runtime/D3D12/Utils/D3D12Helpers.h>

namespace Horizon
{
	class D3D12Device;

	class D3D12Sampler final : public GfxSampler
	{
		friend class D3D12Device;
	public:
		~D3D12Sampler() final;
	};
}