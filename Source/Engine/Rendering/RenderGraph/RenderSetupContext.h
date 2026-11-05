#pragma once

#include <Runtime/Math/Vec2u.h>

namespace Horizon::Engine
{
	struct RenderSetupContext final
	{
		Math::Vec2u colorImageDimension;
		Math::Vec2u shadowMapDimension;
	};
}