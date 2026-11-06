#pragma once

#include <Runtime/Math/Vec2u.h>
#include <Runtime/PAL/Window/WindowFlags.h>

namespace Horizon::Engine
{
	struct WindowParams
	{
		Math::Vec2u windowSize = Math::Vec2u::Zero();
		PAL::WindowFlags flags;
	};
}