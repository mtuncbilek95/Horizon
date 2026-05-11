#pragma once

#include <Runtime/Graphics/RHI/Sampler/GfxSampler.h>

#include <Engine/Core/Context.h>

namespace Horizon
{
	class SamplerContext : public Context<SamplerContext>
	{
	public:
		GfxSampler* GetOrCreate(const GfxSamplerDesc& desc);

	private:
		EngineReport OnInitialize() final;
		void OnFinalize() final;

	private:
		std::unordered_map<GfxSamplerDesc, std::shared_ptr<GfxSampler>> m_samplers;
	};
}