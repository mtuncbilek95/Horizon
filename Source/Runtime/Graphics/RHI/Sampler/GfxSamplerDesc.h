#pragma once

#include <Runtime/Graphics/RHI/Util/PipelineFormat.h>
#include <Runtime/Graphics/RHI/Util/SamplerFormat.h>

namespace Horizon
{
	struct GfxSamplerDesc
	{
		SamplerFilter magFilter = SamplerFilter::Nearest;
		SamplerFilter minFilter = SamplerFilter::Nearest;
		SamplerMipMap mipmapMode = SamplerMipMap::Nearest;
		SamplerAddress addressModeU = SamplerAddress::Repeat;
		SamplerAddress addressModeV = SamplerAddress::Repeat;
		SamplerAddress addressModeW = SamplerAddress::Repeat;
		f32 mipLodBias = 0.f;
		b8 anisotropyEnable = true;
		f32 maxAnisotropy = 16.f;
		b8 compareEnable = true;
		CompareOp compareOp = CompareOp::Always;
		f32 minLod = 0.f;
		f32 maxLod = 1.f;
		BorderColor borderColor = BorderColor::TransparentBlack_Float;

		GfxSamplerDesc& setMagFilter(SamplerFilter filter) { magFilter = filter; return *this; }
		GfxSamplerDesc& setMinFilter(SamplerFilter filter) { minFilter = filter; return *this; }
		GfxSamplerDesc& setMipMapMode(SamplerMipMap mode) { mipmapMode = mode; return *this; }
		GfxSamplerDesc& setAddressModeU(SamplerAddress address) { addressModeU = address; return *this; }
		GfxSamplerDesc& setAddressModeV(SamplerAddress address) { addressModeV = address; return *this; }
		GfxSamplerDesc& setAddressModeW(SamplerAddress address) { addressModeW = address; return *this; }
		GfxSamplerDesc& setMipLodBias(f32 bias) { mipLodBias = bias; return *this; }
		GfxSamplerDesc& setAnisotropyEnable(b8 enable) { anisotropyEnable = enable; return *this; }
		GfxSamplerDesc& setMaxAnisotropy(f32 max) { maxAnisotropy = max; return *this; }
		GfxSamplerDesc& setCompareEnable(b8 enable) { compareEnable = enable; return *this; }
		GfxSamplerDesc& setCompareOp(CompareOp op) { compareOp = op; return *this; }
		GfxSamplerDesc& setMinLod(f32 lod) { minLod = lod; return *this; }
		GfxSamplerDesc& setMaxLod(f32 lod) { maxLod = lod; return *this; }
		GfxSamplerDesc& setBorderColor(BorderColor color) { borderColor = color; return *this; }

		b8 operator==(const GfxSamplerDesc& o) const noexcept 
		{
			return magFilter == o.magFilter && minFilter == o.minFilter
				&& mipmapMode == o.mipmapMode && addressModeU == o.addressModeU
				&& addressModeV == o.addressModeV && addressModeW == o.addressModeW
				&& mipLodBias == o.mipLodBias && anisotropyEnable == o.anisotropyEnable
				&& maxAnisotropy == o.maxAnisotropy && compareEnable == o.compareEnable
				&& compareOp == o.compareOp && minLod == o.minLod
				&& maxLod == o.maxLod && borderColor == o.borderColor;
		}
	};
}

namespace std {
	template<>
	struct hash<Horizon::GfxSamplerDesc> {
		usize operator()(const Horizon::GfxSamplerDesc& d) const noexcept 
		{
			usize seed = 0;
			auto combine = [&seed](auto val)
				{
					seed ^= std::hash<decltype(val)>{}(val)+0x9e3779b9 + (seed << 6) + (seed >> 2);
				};

			combine(static_cast<i32>(d.magFilter));
			combine(static_cast<i32>(d.minFilter));
			combine(static_cast<i32>(d.mipmapMode));
			combine(static_cast<i32>(d.addressModeU));
			combine(static_cast<i32>(d.addressModeV));
			combine(static_cast<i32>(d.addressModeW));
			combine(std::bit_cast<u32>(d.mipLodBias));
			combine(static_cast<i32>(d.anisotropyEnable));
			combine(std::bit_cast<u32>(d.maxAnisotropy));
			combine(static_cast<i32>(d.compareEnable));
			combine(static_cast<i32>(d.compareOp));
			combine(std::bit_cast<u32>(d.minLod));
			combine(std::bit_cast<u32>(d.maxLod));
			combine(static_cast<i32>(d.borderColor));

			return seed;
		}
	};
}