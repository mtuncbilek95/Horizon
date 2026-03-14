#pragma once

#include <Runtime/Graphics/RHI/Object/GfxObject.h>
#include <Runtime/Graphics/RHI/Sampler/GfxSamplerDesc.h>

namespace Horizon
{
	class GfxSampler : public GfxObject
	{
	public:
		GfxSampler(const GfxSamplerDesc& desc, GfxDevice* pDevice);
		virtual ~GfxSampler() override = default;

		virtual void* Sampler() const = 0;

	private:
		GfxSamplerDesc m_desc;
	};
}
