#pragma once

#include <Runtime/Definitions/Handle.h>

namespace Horizon::Engine
{
	struct AssetTag {};
	using AssetHandle = HandleEx<AssetTag>;
}