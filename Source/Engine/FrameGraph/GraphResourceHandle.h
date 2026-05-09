#pragma once

namespace Horizon
{
	template<typename Tag>
	struct ResourceHandle
	{
		u32 id = std::numeric_limits<u32>();
		u32 version = 0;
		
		b8 valid() const { return id != std::numeric_limits<u32>(); }

		b8 operator==(ResourceHandle res) const { return id == res.id; }
		b8 operator!=(ResourceHandle res) const { return id != res.id; }
	};

	struct TextureTag {};
	struct BufferTag {};

	using TextureHandle = ResourceHandle<TextureTag>;
	using BufferHandle = ResourceHandle<BufferTag>;
}