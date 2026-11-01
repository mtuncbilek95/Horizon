#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	HENUM();
	enum class TextureAssetCompression : u8
	{
		None,
		Default,
		HighQuality
	};
}