#pragma once

#include <Runtime/Graphics/RHI/Util/ImageFormat.h>

namespace Horizon
{
	class GfxImage;
	class GfxImageView;

	struct RenderTargetDesc
	{
		Math::Vec2u size;
		ImageUsage usage;
		ImageFormat format;

		b8 operator==(const RenderTargetDesc& other) const
		{
			return size == other.size && usage == other.usage && format == other.format;
		}
	};
}

namespace std
{
	template<>
	struct hash<Horizon::RenderTargetDesc>
	{
		usize operator()(const Horizon::RenderTargetDesc& desc) const
		{
			usize h = 0;
			h ^= hash<u32>{}(desc.size.x) + 0x9e3779b9 + (h << 6) + (h >> 2);
			h ^= hash<u32>{}(desc.size.y) + 0x9e3779b9 + (h << 6) + (h >> 2);
			h ^= hash<u32>{}(static_cast<u32>(desc.format)) + 0x9e3779b9 + (h << 6) + (h >> 2);
			h ^= hash<u32>{}(static_cast<u32>(desc.usage)) + 0x9e3779b9 + (h << 6) + (h >> 2);
			return h;
		}
	};
}