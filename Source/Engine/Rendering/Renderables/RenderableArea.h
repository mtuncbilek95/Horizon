#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/Math/Vec2u.h>

namespace Horizon::Engine
{
	struct RenderableArea
	{
		Math::Vec2u colorImageSize;
		Math::Vec2u shadowMapSize;
	};
}