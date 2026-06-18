#pragma once

namespace Horizon
{
	struct FGTextureHandle
	{
		b8 IsValid() const { return index != kInvalid32; }

		u32 index = kInvalid32;
	};
}