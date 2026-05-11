#pragma once

#include <Runtime/Graphics/RHI/Util/ShaderStage.h>

namespace Horizon
{
    struct GfxShaderDesc
    {
        ShaderStage stage;
        std::span<u32> byteCode;

        GfxShaderDesc& setStage(ShaderStage s) { stage = s; return *this; }
        GfxShaderDesc& setByteCode(std::span<u32> code) { byteCode = code; return *this; }

		b8 operator==(const GfxShaderDesc& other) const
		{
			if (stage != other.stage)
				return false;
			if (byteCode.size() != other.byteCode.size())
				return false;

			return std::memcmp(byteCode.data(), other.byteCode.data(),
				byteCode.size() * sizeof(u32)) == 0;
		}
    };
}

template<>
struct std::hash<Horizon::GfxShaderDesc>
{
	usize operator()(const Horizon::GfxShaderDesc& desc) const noexcept
	{
		usize seed = std::hash<u32>{}(static_cast<u32>(desc.stage));

		for (auto word : desc.byteCode)
			seed ^= std::hash<u32>{}(word)+0x9e3779b9 + (seed << 6) + (seed >> 2);

		return seed;
	}
};