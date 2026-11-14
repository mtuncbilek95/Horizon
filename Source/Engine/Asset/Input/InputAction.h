#pragma once

#include <Runtime/Containers/Vector/List.h>
#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/Math/Vec2f.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Asset
{
	HENUM();
	enum class InputBindingSource : u32
	{
		Key,
		MouseButton,
		MouseDelta,
		MouseWheel,
		GamepadButton,
		GamepadLeftStick,
		GamepadRightStick,
		GamepadLeftTrigger,
		GamepadRightTrigger
	};

	struct InputBinding
	{
		InputBindingSource source = InputBindingSource::Key;
		u64 code = 0;
		Math::Vec2f scale = Math::Vec2f(1.0f, 0.0f);
	};

	struct InputAction
	{
		std::string name;
		List<InputBinding> bindings;
	};
}
