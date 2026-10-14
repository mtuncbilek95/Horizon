#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/Input/GamepadButtons.h>

namespace Horizon::Input
{
	struct GamepadState
	{
		GamepadButtons buttons = GamepadButtons::None;
		f32 leftTrigger = 0.0f;
		f32 rightTrigger = 0.0f;
		f32 leftX = 0.0f;
		f32 leftY = 0.0f;
		f32 rightX = 0.0f;
		f32 rightY = 0.0f;
	};
}