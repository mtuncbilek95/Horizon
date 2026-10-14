#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	HENUM();
	enum class InputSource
	{
		Key,
		MouseButton,
		MouseAxis,
		GamepadButton,
		GamepadAxis
	};

	HENUM();
	enum class InputAxis
	{
		X,
		Y
	};

	HENUM();
	enum class MouseAxisCode
	{
		DeltaX,
		DeltaY,
		WheelX,
		WheelY
	};

	HENUM();
	enum class GamepadAxisCode
	{
		LeftX,
		LeftY,
		RightX,
		RightY,
		LeftTrigger,
		RightTrigger
	};

	HCLASS();
	struct ENGINE_API InputBinding
	{
		HORIZON_DATA_REFLECT(InputBinding);

		HFIELD();
		InputSource source = InputSource::Key;

		HFIELD();
		u32 code = 0;

		HFIELD();
		InputAxis axis = InputAxis::X;

		HFIELD();
		f32 scale = 1.0f;

		HFIELD();
		u32 playerIndex = 0;
	};
}