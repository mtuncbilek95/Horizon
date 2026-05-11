#include "SamplerContext.h"

#include <Runtime/Graphics/RHI/Device/GfxDevice.h>
#include <Engine/Graphics/GfxContext.h>

namespace Horizon
{
	static constexpr f32 NoClamp = 1000.f;

	GfxSampler* SamplerContext::GetOrCreate(const GfxSamplerDesc& desc)
	{
		auto it = m_samplers.find(desc);
		if (it == m_samplers.end())
		{
			auto& device = RequestContext<GfxContext>().Device();

			auto newSamp = device.CreateSampler(desc);
			m_samplers[desc] = newSamp;
			return newSamp.get();
		}

		return it->second.get();
	}

	EngineReport SamplerContext::OnInitialize()
	{
		auto& device = RequestContext<GfxContext>().Device();

		// Linear + Repeat
		GfxSamplerDesc linRep = GfxSamplerDesc()
			.setMagFilter(SamplerFilter::Linear)
			.setMinFilter(SamplerFilter::Linear)
			.setMipMapMode(SamplerMipMap::Linear)
			.setAddressModeU(SamplerAddress::Repeat)
			.setAddressModeV(SamplerAddress::Repeat)
			.setAddressModeW(SamplerAddress::Repeat)
			.setAnisotropyEnable(false)
			.setCompareEnable(false)
			.setMaxLod(NoClamp);
		m_samplers[linRep] = device.CreateSampler(linRep);

		// Linear + Clamp
		GfxSamplerDesc linClamp = GfxSamplerDesc()
			.setMagFilter(SamplerFilter::Linear)
			.setMinFilter(SamplerFilter::Linear)
			.setMipMapMode(SamplerMipMap::Linear)
			.setAddressModeU(SamplerAddress::ClampToEdge)
			.setAddressModeV(SamplerAddress::ClampToEdge)
			.setAddressModeW(SamplerAddress::ClampToEdge)
			.setAnisotropyEnable(false)
			.setCompareEnable(false)
			.setMaxLod(NoClamp);
		m_samplers[linClamp] = device.CreateSampler(linClamp);

		// Linear + Mirror
		GfxSamplerDesc linMir = GfxSamplerDesc()
			.setMagFilter(SamplerFilter::Linear)
			.setMinFilter(SamplerFilter::Linear)
			.setMipMapMode(SamplerMipMap::Linear)
			.setAddressModeU(SamplerAddress::MirroredRepeat)
			.setAddressModeV(SamplerAddress::MirroredRepeat)
			.setAddressModeW(SamplerAddress::MirroredRepeat)
			.setAnisotropyEnable(false)
			.setCompareEnable(false)
			.setMaxLod(NoClamp);
		m_samplers[linMir] = device.CreateSampler(linMir);

		// Nearest + Repeat
		GfxSamplerDesc nearRepeat = GfxSamplerDesc()
			.setMagFilter(SamplerFilter::Nearest)
			.setMinFilter(SamplerFilter::Nearest)
			.setMipMapMode(SamplerMipMap::Nearest)
			.setAddressModeU(SamplerAddress::Repeat)
			.setAddressModeV(SamplerAddress::Repeat)
			.setAddressModeW(SamplerAddress::Repeat)
			.setAnisotropyEnable(false)
			.setCompareEnable(false)
			.setMaxLod(NoClamp);
		m_samplers[nearRepeat] = device.CreateSampler(nearRepeat);

		// Nearest + Clamp
		GfxSamplerDesc nearClamp = GfxSamplerDesc()
			.setMagFilter(SamplerFilter::Nearest)
			.setMinFilter(SamplerFilter::Nearest)
			.setMipMapMode(SamplerMipMap::Nearest)
			.setAddressModeU(SamplerAddress::ClampToEdge)
			.setAddressModeV(SamplerAddress::ClampToEdge)
			.setAddressModeW(SamplerAddress::ClampToEdge)
			.setAnisotropyEnable(false)
			.setCompareEnable(false)
			.setMaxLod(NoClamp);
		m_samplers[nearClamp] = device.CreateSampler(nearClamp);

		// Aniso 4x + Repeat
		GfxSamplerDesc aniso4 = GfxSamplerDesc()
			.setMagFilter(SamplerFilter::Linear)
			.setMinFilter(SamplerFilter::Linear)
			.setMipMapMode(SamplerMipMap::Linear)
			.setAddressModeU(SamplerAddress::Repeat)
			.setAddressModeV(SamplerAddress::Repeat)
			.setAddressModeW(SamplerAddress::Repeat)
			.setAnisotropyEnable(true)
			.setMaxAnisotropy(4.f)
			.setCompareEnable(false)
			.setMaxLod(NoClamp);
		m_samplers[aniso4] = device.CreateSampler(aniso4);

		// Aniso 8x + Repeat
		GfxSamplerDesc aniso8 = GfxSamplerDesc()
			.setMagFilter(SamplerFilter::Linear)
			.setMinFilter(SamplerFilter::Linear)
			.setMipMapMode(SamplerMipMap::Linear)
			.setAddressModeU(SamplerAddress::Repeat)
			.setAddressModeV(SamplerAddress::Repeat)
			.setAddressModeW(SamplerAddress::Repeat)
			.setAnisotropyEnable(true)
			.setMaxAnisotropy(8.f)
			.setCompareEnable(false)
			.setMaxLod(NoClamp);
		m_samplers[aniso8] = device.CreateSampler(aniso8);

		// Aniso 16x + Repeat
		GfxSamplerDesc aniso16 = GfxSamplerDesc()
			.setMagFilter(SamplerFilter::Linear)
			.setMinFilter(SamplerFilter::Linear)
			.setMipMapMode(SamplerMipMap::Linear)
			.setAddressModeU(SamplerAddress::Repeat)
			.setAddressModeV(SamplerAddress::Repeat)
			.setAddressModeW(SamplerAddress::Repeat)
			.setAnisotropyEnable(true)
			.setMaxAnisotropy(16.f)
			.setCompareEnable(false)
			.setMaxLod(NoClamp);
		m_samplers[aniso16] = device.CreateSampler(aniso16);

		// Shadow sampler
		GfxSamplerDesc shadowDesc = GfxSamplerDesc()
			.setMagFilter(SamplerFilter::Linear)
			.setMinFilter(SamplerFilter::Linear)
			.setMipMapMode(SamplerMipMap::Nearest)
			.setAddressModeU(SamplerAddress::ClampToBorder)
			.setAddressModeV(SamplerAddress::ClampToBorder)
			.setAddressModeW(SamplerAddress::ClampToBorder)
			.setAnisotropyEnable(false)
			.setCompareEnable(true)
			.setCompareOp(CompareOp::LesserEqual)
			.setBorderColor(BorderColor::OpaqueBlack_Float)
			.setMaxLod(NoClamp);
		m_samplers[shadowDesc] = device.CreateSampler(shadowDesc);

		return EngineReport();
	}

	void SamplerContext::OnFinalize()
	{}
}